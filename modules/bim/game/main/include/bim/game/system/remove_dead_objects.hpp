// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  class entity_world_map;

  void remove_dead_objects(entt::registry& registry,
                           entity_world_map& entity_map);
}
