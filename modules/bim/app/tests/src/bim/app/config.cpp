// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/config.hpp>

#include <json/value.h>

#include <gtest/gtest.h>

TEST(bim_app_config, game_feature_slot_prices_ok)
{
  Json::Value json;
  json["gs"][0]["h"] = "1.2.3.4:1234";
  json["game-feature-slot-price"][0] = 11;
  json["game-feature-slot-price"][1] = 22;
  json["game-feature-slot-price"][2] = 33;

  static_assert(bim::app::g_game_feature_slot_count == 2);

  std::optional<bim::app::config> config = bim::app::load_config(json);

  ASSERT_TRUE(!!config);

  EXPECT_EQ(11, config->game_feature_slot_price[0]);
  EXPECT_EQ(22, config->game_feature_slot_price[1]);
}

TEST(bim_app_config, game_feature_slot_prices_short)
{
  Json::Value json;
  json["gs"][0]["h"] = "1.2.3.4:1234";
  json["game-feature-slot-price"][0] = 11;

  static_assert(bim::app::g_game_feature_slot_count == 2);

  std::optional<bim::app::config> config = bim::app::load_config(json);

  ASSERT_TRUE(!!config);

  EXPECT_EQ(11, config->game_feature_slot_price[0]);
  EXPECT_EQ(bim::app::config().game_feature_slot_price[1],
            config->game_feature_slot_price[1]);
}

TEST(bim_app_config, game_feature_slot_prices_ignore_non_numeric)
{
  Json::Value json;
  json["gs"][0]["h"] = "1.2.3.4:1234";
  json["game-feature-slot-price"][0] = "11";
  json["game-feature-slot-price"][1] = 22;

  static_assert(bim::app::g_game_feature_slot_count == 2);

  std::optional<bim::app::config> config = bim::app::load_config(json);

  ASSERT_TRUE(!!config);

  EXPECT_EQ(bim::app::config().game_feature_slot_price[0],
            config->game_feature_slot_price[0]);
  EXPECT_EQ(22, config->game_feature_slot_price[1]);
}
