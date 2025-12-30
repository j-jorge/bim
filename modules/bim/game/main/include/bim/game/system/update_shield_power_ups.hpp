// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class entity_world_map;

  void update_shield_power_ups(entt::registry& registry,
                               entity_world_map& entity_map);
}
