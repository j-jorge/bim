// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/flame_animations.hpp>

bool bim::game::flame_animations::is_burning(animation_id id) const
{
  return id == burn;
}
