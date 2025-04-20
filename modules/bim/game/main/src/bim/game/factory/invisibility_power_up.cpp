// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/invisibility_power_up.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::invisibility_power_up_factory(entt::registry& registry,
                                               arena& arena, std::uint8_t x,
                                               std::uint8_t y)
{
  const entt::entity entity = registry.create();

  registry.emplace<invisibility_power_up>(entity);
  registry.emplace<position_on_grid>(entity, x, y);

  arena.put_entity(x, y, entity);

  return entity;
}
