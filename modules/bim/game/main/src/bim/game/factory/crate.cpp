// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/crate.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/crate.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::crate_factory(entt::registry& registry, arena& arena,
                                      std::uint8_t x, std::uint8_t y)
{
  const entt::entity entity = registry.create();

  registry.emplace<crate>(entity);
  registry.emplace<position_on_grid>(entity, x, y);

  arena.put_entity(x, y, entity);
  arena.set_solid(x, y);

  return entity;
}
