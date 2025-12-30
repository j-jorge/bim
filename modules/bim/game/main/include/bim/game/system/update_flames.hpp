// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class entity_world_map;

  void update_flames(entt::registry& registry, entity_world_map& entity_map);
}
