// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flames.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_flames(entt::registry& registry, arena& arena)
{
  registry.view<flame, position_on_grid, timer>().each(
      [&](entt::entity e, const flame& f, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() == 0)
          {
            arena.erase_entity(position.x, position.y);
            registry.destroy(e);
          }
      });
}
