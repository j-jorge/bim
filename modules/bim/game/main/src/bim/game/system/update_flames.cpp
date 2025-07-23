// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flames.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

static void
check_flame_collision(entt::registry& registry, const bim::game::arena& arena,
                      entt::entity e,
                      const bim::game::fractional_position_on_grid& position)
{
  const entt::entity colliding_entity =
      arena.entity_at(position.grid_aligned_x(), position.grid_aligned_y());

  if (colliding_entity == entt::null)
    return;

  if (registry.storage<bim::game::flame>().contains(colliding_entity))
    registry.emplace_or_replace<bim::game::burning>(e);
}

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

  registry.view<fractional_position_on_grid>().each(
      [&](entt::entity e, const fractional_position_on_grid& position) -> void
      {
        check_flame_collision(registry, arena, e, position);
      });
}
