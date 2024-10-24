// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   ulf/decup_ein/rx/base.hpp
/// \author Vincent Hamp
/// \date   24/10/2024

#include <cstdint>
#include <decup/decup.hpp>
#include <optional>
#include <string_view>

namespace ulf::decup_ein::rx {

//
class Base {
public:
  std::optional<uint8_t> receive(uint8_t byte);

private:
  virtual uint8_t transmit(std::span<uint8_t const> bytes) = 0;

  void reset();

  /// IDs 200-205 uses 32b packets, all other IDs use 64b
  static constexpr std::array<uint8_t, 5uz> _ids_which_use_32b{
    200u, 202u, 203u, 204u, 205u};

  decup::Packet _packet{};

  enum : uint8_t {
    Entry,
    Preamble,
    Startbyte,
    Blockcount,
    SecurityByte1,
    SecurityByte2,
    Data,
  } _state{};

  size_t _block_count{};
  uint8_t _decoder_id{};
};

}  // namespace ulf::decup_ein::rx