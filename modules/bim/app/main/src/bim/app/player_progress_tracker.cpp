// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/player_progress_tracker.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/preference/arena_stats.hpp>
#include <bim/app/preference/wallet.hpp>

#include <bim/net/contest_result.hpp>

#include <iscool/preferences/local_preferences.hpp>

bim::app::player_progress_tracker::player_progress_tracker(
    analytics_service& analytics,
    iscool::preferences::local_preferences& local_preferences)
  : m_analytics(analytics)
  , m_preferences(local_preferences)
{}

bim::app::player_progress_tracker::~player_progress_tracker() = default;

void bim::app::player_progress_tracker::game_over_in_public_arena(
    const bim::net::contest_result& result, std::uint8_t local_player_index)
{
  games_in_arena(m_preferences, games_in_arena(m_preferences) + 1);

  if (result.coins_reward != 0)
    add_coins(m_preferences, result.coins_reward);

  if (!result.game_result.has_a_winner())
    {
      coins_transaction(m_analytics, "arena-draw", result.coins_reward);
      return;
    }

  std::int64_t victories = victories_in_arena(m_preferences);
  std::int64_t defeats = defeats_in_arena(m_preferences);

  if (result.game_result.winning_player() == local_player_index)
    {
      ++victories;
      victories_in_arena(m_preferences, victories);
      coins_transaction(m_analytics, "arena-victory", result.coins_reward);
    }
  else
    {
      ++defeats;
      defeats_in_arena(m_preferences, defeats);
      coins_transaction(m_analytics, "arena-defeat", result.coins_reward);
    }
}
