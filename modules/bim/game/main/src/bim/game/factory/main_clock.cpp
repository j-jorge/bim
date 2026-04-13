// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/main_clock.hpp>

#include <bim/game/component/clock.hpp>
#include <bim/game/component/game_clock.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::main_clock_factory(entt::registry& registry)
{
  const entt::entity entity = registry.create();

  registry.emplace<bim::game::clock>(entity, std::chrono::seconds(0));
  registry.emplace<bim::game::game_clock>(entity);

  return entity;
}
