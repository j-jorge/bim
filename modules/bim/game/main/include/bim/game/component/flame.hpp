// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/flame_direction_fwd.hpp>

#include <chrono>

namespace bim::game
{
  struct flame
  {
    flame_direction direction;
    flame_segment segment;
    std::chrono::milliseconds time_to_live;
  };
}
