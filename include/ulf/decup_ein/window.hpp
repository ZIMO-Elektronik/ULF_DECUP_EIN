// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Time window [ms] for each command in
///
/// \file   ulf/decup_ein/ack.hpp
/// \author Jonas Gahlert
/// \date   13/11/2025

#include <cstdint>

namespace ulf::decup_ein::times {

// ZPP
constexpr uint32_t zpp_preamble{0uz};
constexpr uint32_t zpp_cv_read{1uz};
constexpr uint32_t zpp_cv_write{1uz};
constexpr uint32_t zpp_flash_erase{200uz * 1000uz};
constexpr uint32_t zpp_flash_write{1uz};
constexpr uint32_t zpp_decoder_id{1uz};
constexpr uint32_t zpp_crc_or_xor{1uz};

// ZSU
constexpr uint32_t zsu_preamble{0uz};
constexpr uint32_t zsu_decoder_id{1uz};
constexpr uint32_t zsu_block_count{1uz};
constexpr uint32_t zsu_security_bytes{5uz};
constexpr uint32_t zsu_blocks{100uz};

} // namespace ulf::decup_ein::times
