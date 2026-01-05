// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/invisibility_state.hpp>

#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

entt::entity
bim::game::invisibility_state_factory(entt::registry& registry, entt::entity e,
                                      std::chrono::milliseconds duration)
{
  const entt::entity entity = registry.create();

  registry.emplace_or_replace<invisibility_state>(e, entity);
  registry.emplace<timer>(entity, duration);

  return entity;
}
