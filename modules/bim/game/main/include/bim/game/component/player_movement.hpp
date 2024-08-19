// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_movement_fwd.hpp>

namespace bim::game
{
  enum class player_movement : std::uint8_t
  {
    // keep idle at zero such that zero-initialization means no movement.
    idle,
    up,
    down,
    left,
    right
  };

  constexpr player_movement g_max_player_movement = player_movement::right;
}
