// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_id.hpp>

namespace bim::game
{
  struct crate_animations
  {
    bool is_burning(animation_id id) const;

    animation_id idle;
    animation_id burn;
  };
}
