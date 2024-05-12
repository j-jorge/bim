#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  void update_bomb_power_ups(entt::registry& registry, arena& arena);
}
