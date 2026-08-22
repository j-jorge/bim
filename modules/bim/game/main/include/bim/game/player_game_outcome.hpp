// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/player_game_outcome_fwd.hpp>

namespace bim::game
{
  enum class player_game_outcome : std::uint8_t
  {
    defeated,
    kicked,
    draw,
    victory
  };
}
