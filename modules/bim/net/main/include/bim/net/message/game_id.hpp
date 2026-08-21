// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::net
{
  // A game ID, as assigned by the business server.
  using game_id = std::int64_t;

  constexpr game_id not_a_game = 0;
  constexpr game_id pending_game_id = -1;
}
