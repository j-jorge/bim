// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/crushed.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_death_kind.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/factory/player_death_marker.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_players(const context& context,
                               entt::registry& registry)
{
  const player_animations& animations = context.get<const player_animations>();

  registry.view<player>().each(
      [&](entt::entity e, const player& p) -> void
        {
          if (registry.storage<kicked>().contains(e))
            {
              player_death_marker_factory(registry, p.index,
                                          player_death_kind::kicked);
              registry.emplace<dead>(e);
            }
        });

  registry.view<player, burning, animation_state>().each(
      [&](const player& p, animation_state& state) -> void
        {
          if (animations.is_alive(state.model))
            {
              player_death_marker_factory(registry, p.index,
                                          player_death_kind::defeated);
              state.transition_to(animations.burn);
            }
        });

  registry.view<player, crushed, animation_state>().each(
      [&](entt::entity e, const player& p, animation_state& state) -> void
        {
          if (animations.is_alive(state.model))
            {
              player_death_marker_factory(registry, p.index,
                                          player_death_kind::defeated);
              state.transition_to(animations.die);
            }

          registry.erase<crushed>(e);
        });
}
