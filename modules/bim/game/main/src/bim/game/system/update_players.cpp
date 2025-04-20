// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>

#include <entt/entity/registry.hpp>

static void
check_player_collision(entt::registry& registry, const bim::game::arena& arena,
                       entt::entity e,
                       bim::game::fractional_position_on_grid position,
                       const bim::game::player_animations& animations,
                       bim::game::animation_state& state)
{
  const entt::entity colliding_entity =
      arena.entity_at(position.grid_aligned_x(), position.grid_aligned_y());

  if (colliding_entity == entt::null)
    return;

  if (registry.storage<bim::game::flame>().contains(colliding_entity))
    state.transition_to(animations.burn);
}

void bim::game::update_players(const context& context,
                               entt::registry& registry, const arena& arena)
{
  const player_animations& animations = context.get<const player_animations>();

  registry.view<player, fractional_position_on_grid, animation_state, timer>().each(
      [&](entt::entity e, player& p, fractional_position_on_grid position,
          animation_state& state, timer& t) -> void
      {
        p.invisible = t.duration > std::chrono::milliseconds(0);

        if (registry.storage<kicked>().contains(e))
          registry.emplace<dead>(e);
        else if (animations.is_alive(state.model))
          check_player_collision(registry, arena, e, position, animations,
                                 state);
      });
}
