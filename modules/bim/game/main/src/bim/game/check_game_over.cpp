// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/check_game_over.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/game_timer.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/contest_result.hpp>

#include <entt/entity/registry.hpp>

bim::game::contest_result
bim::game::check_game_over(const entt::registry& registry)
{
  int player_count = 0;
  std::uint8_t winner;

  registry.view<player>(entt::exclude<dead>)
      .each(
          [&](const player& p)
          {
            ++player_count;
            winner = p.index;
          });

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
