// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/is_solid.hpp>

#include <bim/game/component/solid.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

bool bim::game::is_solid(const entt::registry& registry,
                         const entity_world_map& entity_map,
                         std::uint8_t arena_x, std::uint8_t arena_y)
{
  const entt::registry::storage_for_type<const solid>* const solids =
      registry.storage<solid>();

  if (solids)
    for (entt::entity e : entity_map.entities_at(arena_x, arena_y))
      if (solids->contains(e))
        return true;

  return false;
}
