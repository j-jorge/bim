// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_power_up_spawners.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <bim/game/factory/power_up.hpp>

#include <entt/entity/registry.hpp>

template <typename PowerUpSpawner>
void bim::game::update_power_up_spawners(entt::registry& registry,
                                         arena& arena)
{
  registry.view<burning, PowerUpSpawner, position_on_grid>().each(
      [&](entt::entity, position_on_grid position) -> void
      {
        power_up_factory<typename PowerUpSpawner::power_up_type>(
            registry, arena, position.x, position.y);
      });
}
