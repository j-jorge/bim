// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/animation_state.hpp>

void bim::game::animation_state::transition_to(animation_id m)
{
  if (model != m)
    {
      model = m;
      elapsed_time = {};
    }
}
