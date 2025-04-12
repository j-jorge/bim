// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_falling_blocks.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>

#include <entt/entity/registry.hpp>

static void seal_falling_block(const bim::game::context& context,
                               entt::registry& registry,
                               bim::game::arena& arena, entt::entity e,
                               const bim::game::position_on_grid& position)
{
  registry.emplace_or_replace<bim::game::dead>(e);

  arena.set_static_wall(position.x, position.y, {});

  const bim::game::player_animations& animations =
      context.get<const bim::game::player_animations>();

  registry
      .view<bim::game::player, bim::game::fractional_position_on_grid,
            bim::game::animation_state>()
      .each(
          [&](entt::entity player_entity, const bim::game::player&,
              const bim::game::fractional_position_on_grid& player_position,
              bim::game::animation_state& state) -> void
          {
            if ((position.x == player_position.grid_aligned_x())
                && (position.y == player_position.grid_aligned_y())
                && animations.is_alive(state.model))
              state.transition_to(animations.die);
          });

  const entt::entity entity_in_arena = arena.entity_at(position.x, position.y);

  if (entity_in_arena == entt::null)
    return;

  arena.erase_entity(position.x, position.y);

  bim::game::timer* const t =
      registry.try_get<bim::game::timer>(entity_in_arena);

  if (t)
    t->duration = {};
  else
    registry.emplace_or_replace<bim::game::dead>(entity_in_arena);
}

void bim::game::update_falling_blocks(const context& context,
                                      entt::registry& registry, arena& arena)
{
  registry.view<timer, falling_block, position_on_grid>().each(
      [&](entt::entity e, const timer& t,
          const position_on_grid& position) -> void
      {
        if (t.duration.count() > 0)
          return;

        seal_falling_block(context, registry, arena, e, position);
      });
}
