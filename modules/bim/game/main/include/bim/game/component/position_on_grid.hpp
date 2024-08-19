// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  struct position_on_grid
  {
    position_on_grid() = default;
    position_on_grid(std::uint8_t x, std::uint8_t y)
      : x(x)
      , y(y)
    {}

    friend bool operator==(position_on_grid lhs, position_on_grid rhs)
    {
      return (lhs.x == rhs.x) && (lhs.y == rhs.y);
    }

    std::uint8_t x;
    std::uint8_t y;
  };
}
