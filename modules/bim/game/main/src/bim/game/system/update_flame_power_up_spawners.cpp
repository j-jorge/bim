// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flame_power_up_spawners.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/power_up.hpp>

#include <entt/entity/registry.hpp>

namespace bim::game
{
  class flame_power_up;
}

void bim::game::update_flame_power_up_spawners(entt::registry& registry,
                                               arena& arena)
{
  registry.view<burning, flame_power_up_spawner, position_on_grid>().each(
      [&](entt::entity, position_on_grid position) -> void
      {
        power_up_factory<flame_power_up>(registry, arena, position.x,
                                         position.y);
      });
}
