// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/update_preferences.hpp>

#include <bim/axmol/app/config.hpp>
#include <bim/axmol/app/preference/arena_stats.hpp>
#include <bim/axmol/app/preference/wallet.hpp>

#include <iscool/preferences/local_preferences.hpp>

void bim::axmol::app::update_preferences(
    iscool::preferences::local_preferences& p, const config& config)
{
  std::int64_t version = p.get_value("version", (std::int64_t)0);

  if (version == 0)
    {
      // Version 1 introduced the coins for each completed game.
      std::int64_t coins = victories_in_arena(p) * config.coins_per_victory
                           + defeats_in_arena(p) * config.coins_per_defeat;

      coins_balance(p, std::max<int64_t>(0, coins));
      ++version;
    }

  p.set_value("version", version);
}
