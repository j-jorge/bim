// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/main_timer.hpp>

#include <bim/game/component/game_timer.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::main_timer_factory(entt::registry& registry,
                                           std::chrono::milliseconds delay)
{
  const entt::entity entity = registry.create();

  registry.emplace<bim::game::timer>(entity, delay);
  registry.emplace<bim::game::game_timer>(entity);

  return entity;
}
