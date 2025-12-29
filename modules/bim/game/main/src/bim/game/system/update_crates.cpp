// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_crates.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_crates(entt::registry& registry, arena& arena)
{
  registry.view<crate, burning, position_on_grid>().each(
      [&](entt::entity e, position_on_grid position) -> void
      {
        registry.emplace<dead>(e);
        arena.erase_entity(position.x, position.y);
      });
}
