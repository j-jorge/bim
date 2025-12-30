// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class entity_world_map;

  template <typename PowerUpSpawner>
  void update_power_up_spawners(entt::registry& registry,
                                entity_world_map& entity_map);
}
