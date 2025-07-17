// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/invincibility_state.hpp>

#include <entt/entity/registry.hpp>

bool bim::game::is_invincible(const entt::registry& registry,
                              entt::entity entity)
{
  const entt::registry::storage_for_type<invincibility_state>* const storage =
      registry.storage<bim::game::invincibility_state>();

  return storage && storage->contains(entity);
}
