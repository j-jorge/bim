// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_id.hpp>

namespace bim::game
{
  struct player_animations
  {
    bool is_alive(animation_id id) const;

    animation_id idle_down;
    animation_id idle_left;
    animation_id idle_right;
    animation_id idle_up;

    animation_id walk_down;
    animation_id walk_left;
    animation_id walk_right;
    animation_id walk_up;

    animation_id burn;
    animation_id die;
  };
}
