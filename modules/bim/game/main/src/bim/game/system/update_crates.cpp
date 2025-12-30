// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_crates.hpp>

#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_crates(entt::registry& registry,
                              entity_world_map& entity_map)
{
  registry.view<crate, burning, position_on_grid>().each(
      [&](entt::entity e, position_on_grid position) -> void
      {
        registry.emplace<dead>(e);
        entity_map.erase_entity(e, position.x, position.y);
      });
}
