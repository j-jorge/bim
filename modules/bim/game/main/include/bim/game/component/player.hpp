// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_direction_fwd.hpp>

#include <cstdint>

namespace bim::game
{
  struct player
  {
    std::uint8_t index;
    player_direction current_direction;
    std::uint8_t bomb_capacity;
    std::uint8_t bomb_available;
    std::uint8_t bomb_strength;
  };
}
