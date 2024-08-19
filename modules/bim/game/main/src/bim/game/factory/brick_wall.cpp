// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/brick_wall.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::brick_wall_factory(entt::registry& registry,
                                           arena& arena, std::uint8_t x,
                                           std::uint8_t y)
{
  const entt::entity entity = registry.create();

  registry.emplace<brick_wall>(entity);
  registry.emplace<position_on_grid>(entity, x, y);

  arena.put_entity(x, y, entity);
  arena.set_solid(x, y);

  return entity;
}
