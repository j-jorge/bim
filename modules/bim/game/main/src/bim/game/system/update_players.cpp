// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_players(const context& context,
                               entt::registry& registry)
{
  const player_animations& animations = context.get<const player_animations>();

  registry.view<player, fractional_position_on_grid, animation_state>().each(
      [&](entt::entity e, const player&, fractional_position_on_grid position,
          animation_state& state) -> void
      {
        if (registry.storage<kicked>().contains(e))
          registry.emplace<dead>(e);
      });

  registry.view<player, burning, animation_state>().each(
      [&](entt::entity e, const player&, animation_state& state) -> void
      {
        if (animations.is_alive(state.model))
          state.transition_to(animations.burn);
      });
}
