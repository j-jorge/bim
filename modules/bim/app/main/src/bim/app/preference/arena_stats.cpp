// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/arena_stats.hpp>

#include <iscool/preferences/local_preferences.hpp>

std::int64_t
bim::app::games_in_arena(const iscool::preferences::local_preferences& p)
{
  return p.get_value("arena_stats.game_count", (std::int64_t)0);
}

void bim::app::games_in_arena(iscool::preferences::local_preferences& p,
                              std::int64_t v)
{
  p.set_value("arena_stats.game_count", v);
}

std::int64_t
bim::app::victories_in_arena(const iscool::preferences::local_preferences& p)
{
  return p.get_value("arena_stats.victory_count", (std::int64_t)0);
}

void bim::app::victories_in_arena(iscool::preferences::local_preferences& p,
                                  std::int64_t v)
{
  p.set_value("arena_stats.victory_count", v);
}

std::int64_t
bim::app::defeats_in_arena(const iscool::preferences::local_preferences& p)
{
  return p.get_value("arena_stats.defeat_count", (std::int64_t)0);
}

void bim::app::defeats_in_arena(iscool::preferences::local_preferences& p,
                                std::int64_t v)
{
  p.set_value("arena_stats.defeat_count", v);
}
