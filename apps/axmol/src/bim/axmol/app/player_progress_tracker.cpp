// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/player_progress_tracker.hpp>

#include <bim/axmol/app/preference/arena_stats.hpp>
#include <bim/axmol/app/preference/feature_flags.hpp>

#include <bim/game/contest_result.hpp>
#include <bim/game/feature_flags.hpp>

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
    return;

  std::int64_t victories = victories_in_arena(preferences);
  std::int64_t defeats = defeats_in_arena(preferences);

  if (result.winning_player() == local_player_index)
    {
      ++victories;
      victories_in_arena(preferences, victories);
    }
  else
    {
      ++defeats;
      defeats_in_arena(preferences, defeats);
    }

  bim::game::feature_flags available_features =
      available_feature_flags(preferences);
  bim::game::feature_flags enabled_features =
      enabled_feature_flags(preferences);

  const std::int64_t rank = 2 * victories + defeats;

  if (!(available_features & bim::game::feature_flags::falling_blocks)
      && (rank >= 2))
    {
      available_features |= bim::game::feature_flags::falling_blocks;
      enabled_features |= bim::game::feature_flags::falling_blocks;
    }

  if (!(available_features & bim::game::feature_flags::invisibility)
      && (rank >= 10))
    {
      available_features |= bim::game::feature_flags::invisibility;
      enabled_features |= bim::game::feature_flags::invisibility;
    }

  if (!(available_features & bim::game::feature_flags::fog_of_war)
      && (rank >= 15))
    {
      available_features |= bim::game::feature_flags::fog_of_war;
      enabled_features |= bim::game::feature_flags::fog_of_war;
    }

  available_feature_flags(preferences, available_features);
  enabled_feature_flags(preferences, enabled_features);
}
