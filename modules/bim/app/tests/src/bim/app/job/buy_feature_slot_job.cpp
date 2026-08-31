// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/buy_feature_slot_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/config.hpp>

#include <bim/business/request_headers.hpp>

#include <gtest/gtest.h>

TEST(bim_app_buy_feature_slot_job, success)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  bim::business::request_headers headers;
  bim::app::config config;
  bim::app::player_profile profile{};

  config.game_feature_slot_price[0] = 3;
  config.game_feature_slot_price[1] = 7;
  profile.coins = 1000;

  http.next_response["client/game-feature/buy-slot"] = { 200,
                                                         Json::objectValue };

  bool bought[bim::app::g_game_feature_slot_count]{};

  bim::app::buy_feature_slot_job job(analytics, headers, config, profile);
  job.connect_to_done(
      [&](std::size_t i)
        {
          EXPECT_FALSE(bought[i]);
          EXPECT_TRUE(profile.slot_availability[i]);

          bought[i] = true;
        });

  job.start(1);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_FALSE(bought[0]);
  EXPECT_TRUE(bought[1]);
  EXPECT_EQ(1000 - 7, profile.coins);
}
