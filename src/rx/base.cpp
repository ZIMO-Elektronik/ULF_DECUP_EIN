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

/// Receive single byte (from e.g. USB)
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::receive(uint8_t byte) {
  return std::invoke(_state, this, byte);
}

/// Reset
///
/// Reset internal state to initial.
///
/// \return std::nullopt
std::optional<uint8_t> Base::reset() {
  _packet.clear();
  _state = &Base::entry;
  _block_count = _decoder_id = 0u;
  return std::nullopt;
}

/// Entry
///
/// Skips additional DECUP_EIN strings should they occur.
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::entry(uint8_t byte) {
  // Ignore entry string which might occur multiple times...
  if ("DECUP_EIN\r"sv.contains(static_cast<char>(byte))) return std::nullopt;
  _state = &Base::preamble;
  return preamble(byte);
}

/// Preamble
///
/// Transmit any preamble, continue with either zpp or zsu.
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::preamble(uint8_t byte) {
  // Still Preamble?
  if (byte == std::to_underlying(decup::Command::Preamble0) ||
      byte == std::to_underlying(decup::Command::Preamble1))
    return pulse_count2response(
      transmit({&byte, sizeof(byte)}, decup::Timeouts::zpp_preamble));
  // Continue with ZPP
  else if (byte < 0x80u) {
    _state = &Base::zpp;
    return zpp(byte);
  }
  // Continue with ZSU
  else {
    _state = &Base::zsuDecoderId;
    return zsuDecoderId(byte);
  }
}

/// ZPP
///
/// \note
/// ---> [Command]
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zpp(uint8_t byte) {
  _packet.clear();
  switch (byte) {
    case 0x01u: _state = &Base::zppReadCv; return zppReadCv(byte);
    case 0x02u: [[fallthrough]];
    case 0x06u: _state = &Base::zppWriteCv; return zppWriteCv(byte);
    case 0x03u: _state = &Base::zppFlashErase; return zppFlashErase(byte);
    case 0x05u: _state = &Base::zppFlashWrite; return zppFlashWrite(byte);
    case 0x04u: _state = &Base::zppDecoderId; return zppDecoderId(byte);
    case 0x07u: _state = &Base::zppCrcXorQuery; return zppCrcXorQuery(byte);
  }
  return std::nullopt;
}

/// ZPP Read Cv
///
/// \note
/// After transmission ---> ZPP
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zppReadCv(uint8_t byte) {
  _packet.push_back(byte);
  if (size(_packet) < 3uz) return std::nullopt;
  else if (size(_packet) == 3uz)
    return pulse_count2response(
      transmit(_packet, decup::Timeouts::zpp_cv_read));
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zpp_cv_read)};
  if (size(_packet) == 3uz + CHAR_BIT - 1uz) _state = &Base::zpp;
  return pulse_count2response(pulse_count);
}

/// ZPP Write Cv
///
/// \note
/// After transmission ---> ZPP
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zppWriteCv(uint8_t byte) {
  _packet.push_back(byte);
  if ((size(_packet) == 5uz && _packet[0uz] == 0x02u) ||
      (size(_packet) == 6uz && _packet[0uz] == 0x06u)) {
    _state = &Base::zpp;
    return pulse_count2response(
      transmit(_packet, decup::Timeouts::zpp_cv_write));
  }
  return std::nullopt;
}

/// ZPP Erase Flash
///
/// \note
/// After transmission ---> ZPP
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zppFlashErase(uint8_t byte) {
  _packet.push_back(byte);
  if (size(_packet) == 4uz && _packet[1uz] == 0x55u && _packet[2uz] == 0xFFu &&
      _packet[3uz] == 0xFFu) {
    _state = &Base::zpp;
    return pulse_count2response(
      transmit(_packet, decup::Timeouts::zpp_flash_erase));
  } else if (size(_packet) >= 4uz)
    _state = &Base::zpp; // Incorrect security bytes
  return std::nullopt;
}

/// ZPP Write Flash
///
/// \note
/// After transmission ---> ZPP
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zppFlashWrite(uint8_t byte) {
  _packet.push_back(byte);
  if (size(_packet) == DECUP_MAX_PACKET_SIZE) {
    _state = &Base::zpp;
    return pulse_count2response(
      transmit(_packet, decup::Timeouts::zpp_flash_write));
  }
  return std::nullopt;
}

/// ZPP Decoder ID
///
/// \note
/// After transmission ---> ZPP
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zppDecoderId(uint8_t byte) {
  _packet.push_back(byte);
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zpp_decoder_id)};
  if (size(_packet) == 1uz + CHAR_BIT - 1uz) _state = &Base::zpp;
  return pulse_count2response(pulse_count);
}

/// ZPP CRC or XOR Query
///
/// \deprecated
/// This command is deprecated, use with care
///
/// \note
/// After transmission ---> ZPP
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zppCrcXorQuery(uint8_t byte) {
  _packet.push_back(byte);
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zpp_crc_or_xor)};
  if (size(_packet) == 1uz + CHAR_BIT - 1uz) _state = &Base::zpp;
  return pulse_count2response(pulse_count);
}

/// ZSU Decoder ID
///
/// \note
/// Double pulse ---> ZSU block count
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zsuDecoderId(uint8_t byte) {
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zsu_decoder_id)};
  if (pulse_count == 2uz) {
    _state = &Base::zsuBlockCount;
    _decoder_id = byte;
  }
  return pulse_count2response(pulse_count);
}

/// ZSU block count
///
/// \note
/// Single pulse ---> ZSU SecurityByte1
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zsuBlockCount(uint8_t byte) {
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zsu_block_count)};
  if (pulse_count == 1uz) {
    _state = &Base::zsuSecurityByte1;
    assert(byte > 8u + 1u);
    auto const data_size{decup::decoder_id2data_size(_decoder_id)};
    auto const bootloader_size{decup::decoder_id2bootloader_size(_decoder_id)};
    // For some reason, for PIC16 decoders the normal calculation results in
    // only half the actual block_count
    auto const factor{data_size == 32uz ? 2uz : 1uz};
    _block_count =
      (((byte + 1u) * 256u - bootloader_size) / data_size) * factor;
  }
  return pulse_count2response(pulse_count);
}

/// ZSU SecurityByte 1
///
/// \note
/// Single pulse ---> ZSU SecurityByte2
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zsuSecurityByte1(uint8_t byte) {
  if (byte != 0x55u) return reset();
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zsu_security_bytes)};
  if (pulse_count == 1uz) _state = &Base::zsuSecurityByte2;
  return pulse_count2response(pulse_count);
}

/// ZSU SecurityByte 2
///
/// \note
/// Single pulse ---> ZSU Blocks
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zsuSecurityByte2(uint8_t byte) {
  if (byte != 0xAAu) return reset();
  auto const pulse_count{
    transmit({&byte, sizeof(byte)}, decup::Timeouts::zsu_security_bytes)};
  if (pulse_count == 1uz) _state = &Base::zsuBlocks;
  return pulse_count2response(pulse_count);
}

/// ZSU Blocks
///
/// Depending on ID, either 32 or 64 Byte blocks are transmitted.
///
/// \note
/// Double pulse ---> Next packet
/// Single pulse ---> Repeat packet
///
/// \param  byte          Byte
/// \retval std::optional No result (yet)
/// \retval uint8_t       Pulse count
std::optional<uint8_t> Base::zsuBlocks(uint8_t byte) {
  _packet.push_back(byte);

  // Not enough bytes
  if (size(_packet) < decup::decoder_id2data_size(_decoder_id) + 2uz)
    return std::nullopt;

  // Whatever happens after that, clear the packet
  gsl::final_action clear_packet{[this] { _packet.clear(); }};

  auto const pulse_count{transmit(_packet, decup::Timeouts::zsu_blocks)};

  // Last packet transmitted successfully
  if (pulse_count == 2uz && !--_block_count) reset();

  return pulse_count2response(pulse_count);
}

} // namespace ulf::decup_ein::rx
