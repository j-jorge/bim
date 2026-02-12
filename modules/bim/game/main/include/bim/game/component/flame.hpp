// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/flame_direction_fwd.hpp>

namespace bim::game
{
  struct flame
  {
    flame_direction direction;
    flame_segment segment;
  };
}
