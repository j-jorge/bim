// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class context;
  class entity_world_map;

  void update_flames(const context& context, entt::registry& registry,
                     entity_world_map& entity_map);
}
