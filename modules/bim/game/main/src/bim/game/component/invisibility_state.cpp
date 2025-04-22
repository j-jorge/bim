// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/invisibility_state.hpp>

#include <entt/entity/registry.hpp>

bool bim::game::is_invisible(const entt::registry& registry,
                             entt::entity entity)
{
  const entt::registry::storage_for_type<invisibility_state>* const storage =
      registry.storage<bim::game::invisibility_state>();

  return storage && storage->contains(entity);
}
