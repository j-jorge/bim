// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/cell_neighborhood_fwd.hpp>

namespace bim::game
{
  enum class cell_neighborhood : std::uint8_t
  {
    none = 0,

    left = (1 << 0),
    right = (1 << 1),
    up = (1 << 2),
    down = (1 << 3),

    up_left = (1 << 4),
    up_right = (1 << 5),
    down_left = (1 << 6),
    down_right = (1 << 7),

    all =
        left | right | up | down | up_left | up_right | down_left | down_right
  };
}
