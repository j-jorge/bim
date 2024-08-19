// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flames.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_flames(entt::registry& registry, arena& arena,
                              std::chrono::milliseconds elapsed_time)
{
  registry.view<flame, position_on_grid>().each(
      [&](entt::entity e, flame& f, position_on_grid position) -> void
      {
        if (elapsed_time >= f.time_to_live)
          {
            registry.destroy(e);
            arena.erase_entity(position.x, position.y);
          }
        else
          f.time_to_live -= elapsed_time;
      });
}
