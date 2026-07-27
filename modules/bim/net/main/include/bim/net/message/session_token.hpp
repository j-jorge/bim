// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>
#include <vector>

namespace bim::net
{
  // This opaque token is sent by the business server to the client the game
  // server. Only the business server knows how to interpret its content, which
  // means that we must stay flexible on its length.
  using session_token = std::vector<std::uint8_t>;
}
