// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/cell_edge_fwd.hpp>

namespace bim::game
{
  enum class cell_edge : std::uint8_t
  {
    none = 0,

    left = (1 << 0),
    right = (1 << 1),
    up = (1 << 2),
    down = (1 << 3),

    all = left | right | up | down
  };

  static_assert((1 << cell_edge_count) - 1 == (int)cell_edge::all);
}
