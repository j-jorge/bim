// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/invincibility_state.hpp>

#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

entt::entity
bim::game::invincibility_state_factory(entt::registry& registry,
                                       entt::entity e,
                                       std::chrono::milliseconds duration)
{
  const entt::entity entity = registry.create();

  registry.emplace_or_replace<invincibility_state>(e, entity);
  registry.emplace<timer>(entity, duration);

  return entity;
}
