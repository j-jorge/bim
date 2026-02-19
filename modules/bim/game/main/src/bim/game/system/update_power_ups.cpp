// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_power_ups.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/power_up.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_power_ups(entt::registry& registry,
                                 entity_world_map& entity_map)
{
  registry.view<power_up, burning, position_on_grid>().each(
      [&](entt::entity e, position_on_grid position) -> void
      {
        entity_map.erase_entity(e, position.x, position.y);
        registry.emplace<dead>(e);
      });
}
