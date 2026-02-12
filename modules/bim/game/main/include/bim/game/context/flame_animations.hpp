// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_id.hpp>

namespace bim::game
{
  struct flame_animations
  {
    bool is_burning(animation_id id) const;

    animation_id warm_up;
    animation_id burn;
    animation_id cool_down;
  };
}
