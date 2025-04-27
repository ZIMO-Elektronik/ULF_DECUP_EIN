// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   ulf/decup_ein/nak.hpp
/// \author Vincent Hamp
/// \date   18/08/2024

#pragma once

#include <cstdint>
#include <optional>
#include "ack.hpp"
#include "nak.hpp"

namespace ulf::decup_ein {

constexpr std::optional<uint8_t> pulse_count2response(size_t pulse_count) {
  if (pulse_count == 1uz) return nak;
  else if (pulse_count == 2uz) return ack;
  else return std::nullopt;
}

} // namespace ulf::decup_ein