// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_movement_fwd.hpp>

#include <cstdint>

namespace bim::game
{
  struct player_action
  {
    player_movement movement;
    bool drop_bomb;
  };
}
