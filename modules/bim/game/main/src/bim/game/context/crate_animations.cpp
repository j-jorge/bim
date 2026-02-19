// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/crate_animations.hpp>

bool bim::game::crate_animations::is_burning(animation_id id) const
{
  return id == burn;
}
