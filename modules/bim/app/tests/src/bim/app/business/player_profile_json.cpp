// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/player_profile_json.hpp>

#include <bim/app/business/player_profile.hpp>

#include <bim/game/feature_flags.hpp>

#include <json/value.h>

#include <gtest/gtest.h>

TEST(bim_app_player_profile, from_json)
{
  Json::Value json;
  json["nickname"] = "the-nickname";
  json["nickname_change_allowed_date"] = "1971-02-03T04:05:06.123456Z";
  json["coins"] = 1;
  json["user_id"] = 11;
  json["feature_slots"][0]["slot_index"] = 1;
  json["feature_slots"][0]["feature"] = "fog_of_war";
  json["available_features"][0] = "fog_of_war";
  json["available_features"][1] = "falling_blocks";
  json["available_features"][2] = "shield";
  json["arena_stats"]["victories"] = 20;
  json["arena_stats"]["defeats"] = 21;
  json["arena_stats"]["draws"] = 22;

  bim::app::player_profile profile = {
    .nickname = "nick",
    .nickname_change_allowed_date = std::chrono::system_clock::now(),
    .coins = 99,
    .user_id = 99,
    .slot_availability = { true, false },
    .slot_feature = { bim::game::feature_flags::invisibility,
                      bim::game::feature_flags{} },
    .available_features = bim::game::feature_flags::invisibility,
    .arena_victories = 99,
    .arena_defeats = 99,
    .arena_draws = 99
  };

  EXPECT_TRUE(bim::app::from_json(profile, json));

  EXPECT_EQ("the-nickname", profile.nickname);

  const std::chrono::system_clock::time_point
      expected_nickname_change_allowed_date =
          std::chrono::sys_days(std::chrono::year_month_day(
              std::chrono::year(1971), std::chrono::February,
              std::chrono::day{ 3 }))
          + std::chrono::hours(4) + std::chrono::minutes(5)
          + std::chrono::seconds(6) + std::chrono::microseconds(123456);

  EXPECT_EQ(expected_nickname_change_allowed_date,
            profile.nickname_change_allowed_date);

  EXPECT_EQ(1, profile.coins);
  EXPECT_EQ(11, profile.user_id);
  EXPECT_FALSE(profile.slot_availability[0]);
  EXPECT_TRUE(profile.slot_availability[1]);
  EXPECT_EQ(bim::game::feature_flags{}, profile.slot_feature[0]);
  EXPECT_EQ(bim::game::feature_flags::fog_of_war, profile.slot_feature[1]);
  EXPECT_EQ(bim::game::feature_flags::fog_of_war
                | bim::game::feature_flags::falling_blocks
                | bim::game::feature_flags::shield,
            profile.available_features);
  EXPECT_EQ(20, profile.arena_victories);
  EXPECT_EQ(21, profile.arena_defeats);
  EXPECT_EQ(22, profile.arena_draws);
}

TEST(bim_app_player_profile, from_json_empty_slot)
{
  Json::Value json;
  json["nickname"] = "the-nickname";
  json["nickname_change_allowed_date"] = "1971-02-03T04:05:06.123456Z";
  json["coins"] = 1;
  json["user_id"] = 11;
  json["feature_slots"][0]["slot_index"] = 0;
  json["feature_slots"][1]["feature"] = Json::nullValue;
  json["feature_slots"][1]["slot_index"] = 1;
  json["feature_slots"][1]["feature"] = "fog_of_war";
  json["available_features"][0] = "fog_of_war";
  json["available_features"][1] = "falling_blocks";
  json["available_features"][2] = "shield";
  json["arena_stats"]["victories"] = 20;
  json["arena_stats"]["defeats"] = 21;
  json["arena_stats"]["draws"] = 22;

  bim::app::player_profile profile = {
    .nickname = "nick",
    .coins = 99,
    .user_id = 99,
    .slot_availability = { true, true },
    .slot_feature = { bim::game::feature_flags::invisibility,
                      bim::game::feature_flags{} },
    .available_features = bim::game::feature_flags::invisibility,
    .arena_victories = 99,
    .arena_defeats = 99,
    .arena_draws = 99
  };

  EXPECT_TRUE(bim::app::from_json(profile, json));

  EXPECT_EQ("the-nickname", profile.nickname);
  EXPECT_EQ(1, profile.coins);
  EXPECT_EQ(11, profile.user_id);
  EXPECT_TRUE(profile.slot_availability[0]);
  EXPECT_TRUE(profile.slot_availability[1]);
  EXPECT_EQ(bim::game::feature_flags{}, profile.slot_feature[0]);
  EXPECT_EQ(bim::game::feature_flags::fog_of_war, profile.slot_feature[1]);
  EXPECT_EQ(bim::game::feature_flags::fog_of_war
                | bim::game::feature_flags::falling_blocks
                | bim::game::feature_flags::shield,
            profile.available_features);
  EXPECT_EQ(20, profile.arena_victories);
  EXPECT_EQ(21, profile.arena_defeats);
  EXPECT_EQ(22, profile.arena_draws);
}
