#pragma once

#include <entt/entity/fwd.hpp>

namespace bm
{
  namespace game
  {
    class arena;

    void generate_basic_level(entt::registry& registry, arena& arena);
  }
}
