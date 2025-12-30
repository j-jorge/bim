// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/wall.hpp>

#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/component/wall.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::wall_factory(entt::registry& registry,
                                     entity_world_map& entity_map,
                                     std::uint8_t x, std::uint8_t y)
{
  const entt::entity entity = registry.create();

  registry.emplace<wall>(entity);
  registry.emplace<position_on_grid>(entity, x, y);
  registry.emplace<solid>(entity);

  entity_map.put_entity(entity, x, y);

  return entity;
}
