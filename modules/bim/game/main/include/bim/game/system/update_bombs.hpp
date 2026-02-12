// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;
  class context;
  class entity_world_map;

  void update_bombs(const context& context, entt::registry& registry,
                    arena& arena, entity_world_map& entity_map);
}
