#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  class dead;

  void remove_dead_objects(entt::registry& registry);
}
