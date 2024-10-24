// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   rx/base.cpp
/// \author Vincent Hamp
/// \date   24/10/2024

#include "rx/base.hpp"
#include "ack.hpp"
#include "nak.hpp"

namespace ulf::decup_ein::rx {

using namespace std::literals;

///
std::optional<uint8_t> Base::receive(uint8_t byte) {
  std::optional<uint8_t> retval{};

  switch (_state) {
    // -.-
    case Entry:
      if ("DECUP_EIN\r"sv.contains(byte)) break;
      _state = Preamble;
      [[fallthrough]];

    //
    case Preamble:
      //
      if (byte == 0xEFu || byte == 0xBFu) {
        transmit({&byte, sizeof(byte)});
        break;
      }
      //
      _state = Startbyte;
      [[fallthrough]];

    // Doppelpuls -> Blockcount
    case Startbyte:
      if (transmit({&byte, sizeof(byte)}) == 2u) {
        retval = ack;
        _state = Blockcount;
        _decoder_id = byte;
      }
      break;

    // Einfachpuls -> SecurityByte1
    case Blockcount:
      if (transmit({&byte, sizeof(byte)}) == 1u) {
        retval = ack;
        _state = SecurityByte1;
        assert(byte > 8u + 1u);
        _block_count =
          (byte - 8u + 1u) *  // bootloader size is 8 blocks
          (256u / (std::ranges::contains(_ids_which_use_32b, _decoder_id)
                     ? 32uz
                     : 64uz));
      }
      break;

    // Einfachpuls -> SecurityByte2
    case SecurityByte1:
      //
      if (byte != 0x55u) reset();
      //
      else if (transmit({&byte, sizeof(byte)}) == 1u) {
        retval = ack;
        _state = SecurityByte2;
      }
      break;

    // Einfachpuls -> Data32/64
    case SecurityByte2:
      if (byte != 0xAAu) reset();
      //
      else if (transmit({&byte, sizeof(byte)}) == 1u) {
        retval = ack;
        _state = Data;
      }
      break;

    // Doppelpuls -> nächstes Paket
    // Einfachpuls -> letztes Paket wiederholen
    case Data:
      _packet.push_back(byte);
      if (size(_packet) <
            (std::ranges::contains(_ids_which_use_32b, _decoder_id) ? 34uz
                                                                    : 66uz) ||
          decup::exor(_packet))
        break;
      else if (transmit({cbegin(_packet), size(_packet)}) == 2u) retval = ack;
      _packet.clear();
      break;
  }

  return retval;
}

///
void Base::reset() {
  _packet.clear();
  _state = Entry;
  _block_count = _decoder_id = 0u;
}

}  // namespace ulf::decup_ein::rx