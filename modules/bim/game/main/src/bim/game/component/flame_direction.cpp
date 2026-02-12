// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/flame_direction.hpp>

#include <bim/assume.hpp>

bool bim::game::is_horizontal(flame_direction d)
{
  return !is_vertical(d);
}

bool bim::game::is_vertical(flame_direction d)
{
  return (int)d & 1;
}

std::string_view bim::game::to_string(flame_direction d)
{
  switch (d)
    {
    case flame_direction::right:
      return "right";
    case flame_direction::down:
      return "down";
    case flame_direction::left:
      return "left";
    case flame_direction::up:
      return "up";
    }

  bim_assume(false);
  return "";
}

std::string_view bim::game::to_string(flame_segment s)
{
  switch (s)
    {
    case flame_segment::tip:
      return "tip";
    case flame_segment::arm:
      return "arm";
    case flame_segment::origin:
      return "origin";
    }

  bim_assume(false);
  return "";
}
