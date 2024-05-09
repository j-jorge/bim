#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  void update_flame_power_up_spawners(entt::registry& registry, arena& arena);
}
