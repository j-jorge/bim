#include <bim/game/component/flame_direction.hpp>

bool bim::game::is_horizontal(flame_direction d)
{
  return !is_vertical(d);
}

bool bim::game::is_vertical(flame_direction d)
{
  return (int)d & 1;
}
