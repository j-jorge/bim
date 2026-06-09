// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/config.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/json/parse_string.hpp>

#include <gtest/gtest.h>

TEST(bim_app_config, load)
{
  const char* const json = R"(
{
  "game_features":
  [
    {"id": "falling_blocks", "coins": 1},
    {"id": "fences", "coins": 2},
    {"id": "fog_of_war", "coins": 3},
    {"id": "invisibility", "coins": 4},
    {"id": "shield", "coins": 5}
  ],
  "game_feature_slots":
  [
    {"index": 0, "coins": 11},
    {"index": 1, "coins": 22}
  ],
  "game_servers":
  [
    "1.2.3.4:1234",
    "4.3.2.1:4321"
  ],
  "shop":
  [
    {"id": "product_1", "coins": 111},
    {"id": "product_2", "coins": 222},
    {"id": "product_3", "coins": 333},
    {"id": "product_4", "coins": 444}
  ],
  "misc":
  {
    "most_recent_version": 1111,
    "version_update_interval": 2222
  }
}
)";

  const std::optional<bim::app::config> config =
      bim::app::load_config(iscool::json::parse_string(json));

  ASSERT_TRUE(!!config);

  EXPECT_EQ(
      1, config->game_feature_price[bim::game::feature_flags::falling_blocks]);
  EXPECT_EQ(2, config->game_feature_price[bim::game::feature_flags::fences]);
  EXPECT_EQ(3,
            config->game_feature_price[bim::game::feature_flags::fog_of_war]);
  EXPECT_EQ(
      4, config->game_feature_price[bim::game::feature_flags::invisibility]);
  EXPECT_EQ(5, config->game_feature_price[bim::game::feature_flags::shield]);

  EXPECT_EQ(11, config->game_feature_slot_price[0]);
  EXPECT_EQ(22, config->game_feature_slot_price[1]);

  EXPECT_EQ("1.2.3.4:1234", config->game_server);

  ASSERT_EQ(4, config->shop_products.size());
  ASSERT_EQ(config->shop_products.size(), config->shop_product_coins.size());

  EXPECT_EQ("product_1", config->shop_products[0]);
  EXPECT_EQ(111, config->shop_product_coins[0]);

  EXPECT_EQ("product_2", config->shop_products[1]);
  EXPECT_EQ(222, config->shop_product_coins[1]);

  EXPECT_EQ("product_3", config->shop_products[2]);
  EXPECT_EQ(333, config->shop_product_coins[2]);

  EXPECT_EQ("product_4", config->shop_products[3]);
  EXPECT_EQ(444, config->shop_product_coins[3]);

  EXPECT_EQ(1111, config->most_recent_version);
  EXPECT_EQ(2222, config->version_update_interval.count());
}

TEST(bim_app_config, load_tolerance)
{
  // Unknown game features should be ignored.
  // Extra game feature slots should be ignored.
  // Empty game server list is allowed.

  const char* const json = R"(
{
  "game_features":
  [
    {"id": "falling_blocks", "coins": 1},
    {"id": "fences", "coins": 2},
    {"id": "foobar", "coins": 3000},
    {"id": "invisibility", "coins": 4},
    {"id": "shield", "coins": 5}
  ],
  "game_feature_slots":
  [
    {"index": 10, "coins": 11},
    {"index": 1, "coins": 22}
  ],
  "game_servers": [],
  "shop":
  [
    {"id": "product_1", "coins": 111},
    {"id": "product_2", "coins": 222},
    {"id": "product_3", "coins": 333},
    {"id": "product_4", "coins": 444}
  ],
  "misc": {}
}
)";

  const bim::app::config default_config;

  const std::optional<bim::app::config> config =
      bim::app::load_config(iscool::json::parse_string(json));

  ASSERT_TRUE(!!config);

  EXPECT_EQ(
      1, config->game_feature_price[bim::game::feature_flags::falling_blocks]);
  EXPECT_EQ(2, config->game_feature_price[bim::game::feature_flags::fences]);
  EXPECT_EQ(
      default_config.game_feature_price[bim::game::feature_flags::fog_of_war],
      config->game_feature_price[bim::game::feature_flags::fog_of_war]);
  EXPECT_EQ(
      4, config->game_feature_price[bim::game::feature_flags::invisibility]);
  EXPECT_EQ(5, config->game_feature_price[bim::game::feature_flags::shield]);

  EXPECT_EQ(default_config.game_feature_slot_price[0],
            config->game_feature_slot_price[0]);
  EXPECT_EQ(22, config->game_feature_slot_price[1]);

  EXPECT_EQ(default_config.game_server, config->game_server);

  ASSERT_EQ(4, config->shop_products.size());
  ASSERT_EQ(config->shop_products.size(), config->shop_product_coins.size());

  EXPECT_EQ("product_1", config->shop_products[0]);
  EXPECT_EQ(111, config->shop_product_coins[0]);

  EXPECT_EQ("product_2", config->shop_products[1]);
  EXPECT_EQ(222, config->shop_product_coins[1]);

  EXPECT_EQ("product_3", config->shop_products[2]);
  EXPECT_EQ(333, config->shop_product_coins[2]);

  EXPECT_EQ("product_4", config->shop_products[3]);
  EXPECT_EQ(444, config->shop_product_coins[3]);

  EXPECT_EQ(default_config.most_recent_version, config->most_recent_version);
  EXPECT_EQ(default_config.version_update_interval.count(),
            config->version_update_interval.count());
}
