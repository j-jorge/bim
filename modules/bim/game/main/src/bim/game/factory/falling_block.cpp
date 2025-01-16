// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/falling_block.hpp>

#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::falling_block_factory(entt::registry& registry,
                                              const position_on_grid& position,
                                              std::chrono::milliseconds delay)
{
  const entt::entity entity = registry.create();

  registry.emplace<bim::game::timer>(entity, delay);
  registry.emplace<bim::game::position_on_grid>(entity, position);
  registry.emplace<bim::game::falling_block>(entity);

  return entity;
}
