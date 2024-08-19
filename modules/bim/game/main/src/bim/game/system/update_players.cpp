// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>

#include <entt/entity/registry.hpp>

static void
check_player_collision(entt::registry& registry, const bim::game::arena& arena,
                       entt::entity e,
                       bim::game::fractional_position_on_grid position)
{
  const entt::entity colliding_entity =
      arena.entity_at(position.grid_aligned_x(), position.grid_aligned_y());

  if (colliding_entity == entt::null)
    return;

  if (registry.storage<bim::game::flame>().contains(colliding_entity))
    registry.emplace<bim::game::dead>(e);
}

void bim::game::update_players(entt::registry& registry, const arena& arena)
{
  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity e, const player&,
          fractional_position_on_grid position) -> void
      {
        check_player_collision(registry, arena, e, position);
      });
}
