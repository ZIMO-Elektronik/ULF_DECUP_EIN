// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   rx/base.cpp
/// \author Vincent Hamp
/// \date   24/10/2024

#include "rx/base.hpp"
#include <climits>
#include <cstdio>
#include <functional>
#include <gsl/util>
#include "ack.hpp"
#include "nak.hpp"
#include "pulse_count2response.hpp"

namespace ulf::decup_ein::rx {

using namespace std::literals;

///
std::optional<uint8_t> Base::receive(uint8_t byte) {
  return std::invoke(_state, this, byte);
}

///
std::optional<uint8_t> Base::entry(uint8_t byte) {
  // Ignore entry string which might occur multiple times...
  if ("DECUP_EIN\r"sv.contains(static_cast<char>(byte))) return std::nullopt;
  _state = &Base::preamble;
  return preamble(byte);
}

///
std::optional<uint8_t> Base::preamble(uint8_t byte) {
  //
  if (byte == std::to_underlying(decup::Command::Preamble0) ||
      byte == std::to_underlying(decup::Command::Preamble1))
    return pulse_count2response(transmit({&byte, sizeof(byte)}));
  //
  else if (byte < 0x80u) {
    _state = &Base::zpp;
    return zpp(byte);
  }
  //
  else {
    _state = &Base::zsuDecoderId;
    return zsuDecoderId(byte);
  }
}

/// ZPP Klump hat fast sowas wie Kommandos...
std::optional<uint8_t> Base::zpp(uint8_t byte) {
  _packet.clear();
  switch (byte) {
    case 0x01u: _state = &Base::zppReadCv; return zppReadCv(byte);
    case 0x02u: [[fallthrough]];
    case 0x06u: _state = &Base::zppWriteCv; return zppWriteCv(byte);
    case 0x03u: [[fallthrough]];
    case 0x05u: _state = &Base::zppFlash; return zppFlash(byte);
    case 0x04u: _state = &Base::zppDecoderId; return zppDecoderId(byte);
    case 0x07u: _state = &Base::zppCrcXorQuery; return zppCrcXorQuery(byte);
  }
  return std::nullopt;
}

/// \todo
std::optional<uint8_t> Base::zppReadCv(uint8_t byte) {
  _packet.push_back(byte);
  if (size(_packet) < 3uz) return std::nullopt;
  else if (size(_packet) == 3uz) return pulse_count2response(transmit(_packet));
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (size(_packet) == 3uz + CHAR_BIT - 1uz) _state = &Base::zpp;
  return pulse_count2response(pulse_count);
}

/// \todo
std::optional<uint8_t> Base::zppWriteCv(uint8_t byte) {
  _packet.push_back(byte);
  if ((size(_packet) == 5uz && _packet[0uz] == 0x02u) ||
      (size(_packet) == 6uz && _packet[0uz] == 0x06u)) {
    _state = &Base::zpp;
    return pulse_count2response(transmit(_packet));
  }
  return std::nullopt;
}

/// \todo
std::optional<uint8_t> Base::zppFlash(uint8_t byte) {
  _packet.push_back(byte);
  if ((size(_packet) == 4uz && _packet[0uz] == 0x03u && _packet[1uz] == 0x55u &&
       _packet[2uz] == 0xFFu && _packet[3uz] == 0xFFu) ||
      size(_packet) == DECUP_MAX_PACKET_SIZE) {
    _state = &Base::zpp;
    return pulse_count2response(transmit(_packet));
  }
  return std::nullopt;
}

/// \todo
std::optional<uint8_t> Base::zppDecoderId(uint8_t byte) {
  _packet.push_back(byte);
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (size(_packet) == 1uz + CHAR_BIT - 1uz) _state = &Base::zpp;
  return pulse_count2response(pulse_count);
}

/// \todo
std::optional<uint8_t> Base::zppCrcXorQuery(uint8_t byte) {
  _packet.push_back(byte);
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (size(_packet) == 1uz + CHAR_BIT - 1uz) _state = &Base::zpp;
  return pulse_count2response(pulse_count);
}

/// Doppelpuls -> Blockcount
std::optional<uint8_t> Base::zsuDecoderId(uint8_t byte) {
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (pulse_count == 2u) {
    _state = &Base::zsuBlockCount;
    _decoder_id = byte;
  }
  return pulse_count2response(pulse_count);
}

/// Einfachpuls -> SecurityByte1
std::optional<uint8_t> Base::zsuBlockCount(uint8_t byte) {
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (pulse_count == 1u) {
    _state = &Base::zsuSecurityByte1;
    assert(byte > 8u + 1u);
    _block_count = (byte - 8u + 1u) * // bootloader size is 8 blocks
                   (256u / decup::decoder_id2data_size(_decoder_id));
  }
  return pulse_count2response(pulse_count);
}

// Einfachpuls -> SecurityByte2
std::optional<uint8_t> Base::zsuSecurityByte1(uint8_t byte) {
  if (byte != 0x55u) return reset();
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (pulse_count == 1u) _state = &Base::zsuSecurityByte2;
  return pulse_count2response(pulse_count);
}

/// Einfachpuls -> Data32/64
std::optional<uint8_t> Base::zsuSecurityByte2(uint8_t byte) {
  if (byte != 0xAAu) return reset();
  auto const pulse_count{transmit({&byte, sizeof(byte)})};
  if (pulse_count == 1u) _state = &Base::zsuBlocks;
  return pulse_count2response(pulse_count);
}

// Doppelpuls -> nächstes Paket
// Einfachpuls -> letztes Paket wiederholen
std::optional<uint8_t> Base::zsuBlocks(uint8_t byte) {
  _packet.push_back(byte);

  // Not enough bytes
  if (size(_packet) < decup::decoder_id2data_size(_decoder_id) + 2uz)
    return std::nullopt;

  // Whatever happens after that, clear the packet
  gsl::final_action clear_packet{[this] { _packet.clear(); }};

  auto const pulse_count{transmit(_packet)};

  // Last packet transmitted successfully
  if (pulse_count == 2u && !--_block_count) reset();

  return pulse_count2response(pulse_count);
}

///
std::optional<uint8_t> Base::reset() {
  _packet.clear();
  _state = &Base::entry;
  _block_count = _decoder_id = 0u;
  return std::nullopt;
}

} // namespace ulf::decup_ein::rx
