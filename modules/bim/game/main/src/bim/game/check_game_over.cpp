// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/check_game_over.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/game_timer.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>

#include <entt/entity/registry.hpp>

bim::game::contest_result
bim::game::check_game_over(const context& context,
                           const entt::registry& registry)
{
  const bim::game::player_animations& animations =
      context.get<const bim::game::player_animations>();

  int player_count = 0;
  std::uint8_t winner;
  bool one_player_is_dying = false;

  registry.view<player, animation_state>(entt::exclude<dead>)
      .each(
          [&](const player& p, const animation_state& state)
          {
            ++player_count;
            winner = p.index;

            one_player_is_dying |= !animations.is_alive(state.model);
          });

  if (one_player_is_dying)
    return contest_result::create_still_running();

  switch (player_count)
    {
    case 0:
      return contest_result::create_draw();
    case 1:
      return contest_result::create_game_over(winner);
    }

  bool time_is_up = false;

  registry.view<timer, game_timer>().each(
      [&](const timer& t)
      {
        if (t.duration.count() == 0)
          time_is_up = true;
      });

  if (time_is_up)
    return contest_result::create_draw();

  return contest_result::create_still_running();
}
