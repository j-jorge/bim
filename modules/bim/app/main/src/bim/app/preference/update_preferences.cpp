// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/update_preferences.hpp>

#include <bim/app/config.hpp>
#include <bim/app/preference/arena_stats.hpp>
#include <bim/app/preference/feature_flags.hpp>
#include <bim/app/preference/wallet.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/preferences/local_preferences.hpp>

static void
update_preferences_v0_to_v1(iscool::preferences::local_preferences& p,
                            const bim::app::config& config)
{
  // Version 1 introduced the coins for each completed game. It was then
  // decided by the game server and removed from the configuration. We apply
  // the last used values here.
  std::int64_t coins = bim::app::victories_in_arena(p) * 50
                       + bim::app::defeats_in_arena(p) * 10;

  // Game features are purchased with coins.
  const bim::game::feature_flags features =
      (bim::game::feature_flags)p.get_value("feature_flags.available",
                                            (std::int64_t)0);

  if (!!(features & bim::game::feature_flags::falling_blocks))
    coins -=
        config.game_feature_price[bim::game::feature_flags::falling_blocks];

  if (!!(features & bim::game::feature_flags::invisibility))
    coins -= config.game_feature_price[bim::game::feature_flags::invisibility];

  if (!!(features & bim::game::feature_flags::fog_of_war))
    coins -= config.game_feature_price[bim::game::feature_flags::fog_of_war];

  bim::app::coins_balance(p, std::max<int64_t>(0, coins));

  p.set_value("version", (std::int64_t)1);
}

static void
update_preferences_v1_to_v2(iscool::preferences::local_preferences& p,
                            const bim::app::config& config)
{
  // v2 removed the enabled flags mask and introduced the feature flag slots.
  const bim::game::feature_flags enabled =
      (bim::game::feature_flags)p.get_value("feature_flags.enabled",
                                            (std::int64_t)0);

  bim::game::feature_flags slot_0{};

  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    if (!!(enabled & f))
      {
        slot_0 = f;
        break;
      }

  bim::app::available_feature_slot(p, 0, true);
  bim::app::feature_flag_in_slot(p, 0, slot_0);

  p.set_value("version", (std::int64_t)2);
}

void bim::app::update_preferences(iscool::preferences::local_preferences& p,
                                  const config& config)
{
  switch (p.get_value("version", (std::int64_t)0))
    {
    case 0:
      update_preferences_v0_to_v1(p, config);
      [[fallthrough]];
    case 1:
      update_preferences_v1_to_v2(p, config);
    }
}
