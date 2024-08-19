// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <fpm/fixed.hpp>

namespace bim::game
{
  struct fractional_position_on_grid
  {
    using value_type = fpm::fixed<std::uint8_t, std::uint16_t, 4>;

  public:
    fractional_position_on_grid();
    fractional_position_on_grid(value_type x, value_type y);
    fractional_position_on_grid(std::uint8_t x, std::uint8_t y);

    std::uint8_t grid_aligned_x() const;
    std::uint8_t grid_aligned_y() const;

    float x_float() const;
    float y_float() const;

  public:
    value_type x;
    value_type y;
  };
}
