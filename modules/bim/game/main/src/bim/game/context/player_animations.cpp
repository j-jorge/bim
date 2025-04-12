// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/player_animations.hpp>

bool bim::game::player_animations::is_alive(animation_id id) const
{
  return (id != burn) && (id != die);
}
