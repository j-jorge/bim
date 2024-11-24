// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/fractional_position_on_grid.hpp>

#include <fpm/math.hpp>

#include <cassert>

static constexpr bim::game::fractional_position_on_grid::value_type g_quarter =
    bim::game::fractional_position_on_grid::value_type(1) / 4;
static constexpr bim::game::fractional_position_on_grid::value_type
    g_three_quarters =
        bim::game::fractional_position_on_grid::value_type(3) / 4;

bim::game::fractional_position_on_grid::fractional_position_on_grid() =
    default;

bim::game::fractional_position_on_grid::fractional_position_on_grid(
    value_type x, value_type y)
  : x(x)
  , y(y)
{}

bim::game::fractional_position_on_grid::fractional_position_on_grid(
    std::uint8_t x, std::uint8_t y)
  : x(x)
  , y(y)
{}

std::uint8_t bim::game::fractional_position_on_grid::grid_aligned_x() const
{
  return (std::uint8_t)x;
}

std::uint8_t bim::game::fractional_position_on_grid::grid_aligned_y() const
{
  return (std::uint8_t)y;
}

float bim::game::fractional_position_on_grid::x_float() const
{
  return (float)x;
}

float bim::game::fractional_position_on_grid::y_float() const
{
  return (float)y;
}
