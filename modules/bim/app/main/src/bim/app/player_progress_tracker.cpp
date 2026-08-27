// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/player_progress_tracker.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/business/player_profile.hpp>

#include <bim/net/contest_result.hpp>

bim::app::player_progress_tracker::player_progress_tracker(
    analytics_service& analytics)
  : m_analytics(analytics)
{}

void bim::app::player_progress_tracker::game_over_in_public_arena(
    const bim::net::contest_result& result, std::uint8_t local_player_index)
{
  if (!result.game_result.has_a_winner())
    coins_transaction(m_analytics, "arena-draw", result.coins_reward);
  else if (result.game_result.winning_player() == local_player_index)
    coins_transaction(m_analytics, "arena-victory", result.coins_reward);
  else
    coins_transaction(m_analytics, "arena-defeat", result.coins_reward);
}
