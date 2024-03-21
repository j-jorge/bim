#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  void update_players(entt::registry& registry, const arena& arena);
}
