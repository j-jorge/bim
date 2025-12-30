// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;
  class context;
  class entity_world_map;

  void dump_arena(const arena& arena, const entity_world_map& entity_map,
                  const context& context, const entt::registry& registry);
}
