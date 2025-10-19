// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/update_preferences.hpp>

#include <bim/app/config.hpp>
#include <bim/app/preference/arena_stats.hpp>
#include <bim/app/preference/feature_flags.hpp>
#include <bim/app/preference/wallet.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/preferences/local_preferences.hpp>

void bim::app::update_preferences(iscool::preferences::local_preferences& p,
                                  const config& config)
{
  std::int64_t version = p.get_value("version", (std::int64_t)0);

  if (version == 0)
    {
      // Version 1 introduced the coins for each completed game.
      std::int64_t coins = victories_in_arena(p) * config.coins_per_victory
                           + defeats_in_arena(p) * config.coins_per_defeat;

      // Game features are purchased with coins.
      const bim::game::feature_flags features = available_feature_flags(p);

      if (!!(features & bim::game::feature_flags::falling_blocks))
        coins -=
            config
                .game_feature_price[bim::game::feature_flags::falling_blocks];

      if (!!(features & bim::game::feature_flags::invisibility))
        coins -=
            config.game_feature_price[bim::game::feature_flags::invisibility];

      if (!!(features & bim::game::feature_flags::fog_of_war))
        coins -=
            config.game_feature_price[bim::game::feature_flags::fog_of_war];

      coins_balance(p, std::max<int64_t>(0, coins));
      ++version;
    }

  p.set_value("version", version);
}
