// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/arena_reduction.hpp>

#include <bim/game/component/arena_reduction_state.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

entt::entity
bim::game::arena_reduction_factory(entt::registry& registry,
                                   std::chrono::milliseconds delay)
{
  const entt::entity entity = registry.create();

  registry.emplace<arena_reduction_state>(entity, 0);
  registry.emplace<timer>(entity, delay);

  return entity;
}
