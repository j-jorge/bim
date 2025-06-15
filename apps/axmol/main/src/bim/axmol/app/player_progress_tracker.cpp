// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/player_progress_tracker.hpp>

#include <bim/axmol/app/config.hpp>
#include <bim/axmol/app/preference/arena_stats.hpp>
#include <bim/axmol/app/preference/wallet.hpp>

#include <bim/game/contest_result.hpp>

#include <iscool/preferences/local_preferences.hpp>

bim::axmol::app::player_progress_tracker::player_progress_tracker(
    const context& context)
  : m_context(context)
{}

bim::axmol::app::player_progress_tracker::~player_progress_tracker() = default;

void bim::axmol::app::player_progress_tracker::game_over_in_public_arena(
    const bim::game::contest_result& result, std::uint8_t local_player_index)
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  games_in_arena(preferences, games_in_arena(preferences) + 1);

  if (!result.has_a_winner())
    {
      add_coins(preferences, m_context.get_config()->coins_per_draw);
      return;
    }

  std::int64_t victories = victories_in_arena(preferences);
  std::int64_t defeats = defeats_in_arena(preferences);

  if (result.winning_player() == local_player_index)
    {
      ++victories;
      victories_in_arena(preferences, victories);
      add_coins(preferences, m_context.get_config()->coins_per_victory);
    }
  else
    {
      ++defeats;
      defeats_in_arena(preferences, defeats);
      add_coins(preferences, m_context.get_config()->coins_per_defeat);
    }
}
