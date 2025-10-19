// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/player_progress_tracker.hpp>

#include <bim/app/config.hpp>
#include <bim/app/preference/arena_stats.hpp>
#include <bim/app/preference/wallet.hpp>

#include <bim/game/contest_result.hpp>

#include <iscool/preferences/local_preferences.hpp>

bim::app::player_progress_tracker::player_progress_tracker(
    iscool::preferences::local_preferences& local_preferences,
    const config& config)
  : m_preferences(local_preferences)
  , m_config(config)
{}

bim::app::player_progress_tracker::~player_progress_tracker() = default;

void bim::app::player_progress_tracker::game_over_in_public_arena(
    const bim::game::contest_result& result, std::uint8_t local_player_index)
{
  games_in_arena(m_preferences, games_in_arena(m_preferences) + 1);

  if (!result.has_a_winner())
    {
      add_coins(m_preferences, m_config.coins_per_draw);
      return;
    }

  std::int64_t victories = victories_in_arena(m_preferences);
  std::int64_t defeats = defeats_in_arena(m_preferences);

  if (result.winning_player() == local_player_index)
    {
      ++victories;
      victories_in_arena(m_preferences, victories);
      add_coins(m_preferences, m_config.coins_per_victory);
    }
  else
    {
      ++defeats;
      defeats_in_arena(m_preferences, defeats);
      add_coins(m_preferences, m_config.coins_per_defeat);
    }
}
