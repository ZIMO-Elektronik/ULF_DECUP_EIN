// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   ulf/decup_ein/rx/base.hpp
/// \author Vincent Hamp
/// \date   24/10/2024

#include <cstdint>
#include <decup/decup.hpp>
#include <optional>
#include <string_view>

namespace ulf::decup_ein::rx {

/// Rx Base class
///
/// \details Pass data into the \ref receive(uint8_t) method to use. Doing this
/// may result in a call to transmit
///
/// Calling reset() resets the internal state
class Base {
public:
  /// Dtor
  virtual constexpr ~Base() = default;

  std::optional<uint8_t> receive(uint8_t byte);
  std::optional<uint8_t> reset();

private:
  /// Transmit bytes
  ///
  /// \param bytes    Bytes
  /// \param timeout  Response timeout [us]
  /// \return         Pulse count
  virtual uint8_t transmit(std::span<uint8_t const> bytes,
                           uint32_t timeout) = 0;

  std::optional<uint8_t> entry(uint8_t byte);
  std::optional<uint8_t> preamble(uint8_t byte);

  std::optional<uint8_t> zpp(uint8_t byte);
  std::optional<uint8_t> zppReadCv(uint8_t byte);
  std::optional<uint8_t> zppWriteCv(uint8_t byte);
  std::optional<uint8_t> zppFlashErase(uint8_t byte);
  std::optional<uint8_t> zppFlashWrite(uint8_t byte);
  std::optional<uint8_t> zppDecoderId(uint8_t byte);
  std::optional<uint8_t> zppCrcXorQuery(uint8_t byte);

  std::optional<uint8_t> zsuDecoderId(uint8_t byte);
  std::optional<uint8_t> zsuBlockCount(uint8_t byte);
  std::optional<uint8_t> zsuSecurityByte1(uint8_t byte);
  std::optional<uint8_t> zsuSecurityByte2(uint8_t byte);
  std::optional<uint8_t> zsuBlocks(uint8_t byte);

  decup::Packet _packet{};
  std::optional<uint8_t> (Base::*_state)(uint8_t){&Base::entry};
  size_t _block_count{};
  uint8_t _decoder_id{};
};

} // namespace ulf::decup_ein::rx
