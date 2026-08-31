// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/buy_feature_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/config.hpp>

#include <bim/business/request_headers.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <gtest/gtest.h>

TEST(bim_app_buy_feature_job, success)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  bim::business::request_headers headers;
  bim::app::config config;
  bim::app::player_profile profile{};

  config.game_feature_price[bim::game::feature_flags::fog_of_war] = 3;
  config.game_feature_price[bim::game::feature_flags::shield] = 7;
  config.game_feature_price[bim::game::feature_flags::falling_blocks] = 11;
  profile.coins = 1000;

  http.next_response["client/game-feature/buy-feature"] = {
    200, Json::objectValue
  };

  bim::game::feature_flags done_flags{};
  bim::app::buy_feature_job job(analytics, headers, config, profile);
  job.connect_to_done(
      [&](bim::game::feature_flags f)
        {
          http.next_response["client/game-feature/buy-feature"] = {
            200, Json::objectValue
          };

          EXPECT_FALSE(!!(done_flags & f));
          EXPECT_TRUE(!!(profile.available_features & f));

          done_flags |= f;
        });

  job.start(bim::game::feature_flags::falling_blocks);
  job.start(bim::game::feature_flags::shield);
  job.start(bim::game::feature_flags::fog_of_war);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_EQ(bim::game::feature_flags::fog_of_war
                | bim::game::feature_flags::shield
                | bim::game::feature_flags::falling_blocks,
            done_flags);
  EXPECT_EQ(1000 - 11 - 7 - 3, profile.coins);
}
