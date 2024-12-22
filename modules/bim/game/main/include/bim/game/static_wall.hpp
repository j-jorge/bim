// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/cell_neighborhood_fwd.hpp>

#include <cstdint>

namespace bim::game
{
  struct static_wall
  {
    std::uint8_t x;
    std::uint8_t y;
    cell_neighborhood neighbors;
  };
}
