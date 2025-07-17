// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/shield.hpp>

#include <entt/entity/registry.hpp>

bool bim::game::has_shield(const entt::registry& registry, entt::entity e)
{
  const entt::registry::storage_for_type<shield>* const storage =
      registry.storage<shield>();

  return storage && storage->contains(e);
}
