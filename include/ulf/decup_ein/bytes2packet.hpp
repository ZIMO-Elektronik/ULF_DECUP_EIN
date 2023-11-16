// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   ulf/decup_ein/bytes2packet.hpp
/// \author Vincent Hamp
/// \date   12/08/2024

namespace decup {

using Packet = ztl::inplace_vector<uint8_t, DCC_MAX_PACKET_SIZE>;

}  // namespace decup
