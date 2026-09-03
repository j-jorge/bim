// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/consume_game_reward_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/player_profile.hpp>

#include <bim/business/request_headers.hpp>

#include <gtest/gtest.h>

TEST(bim_app_consume_game_reward_job, success)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  bim::business::request_headers headers;
  bim::app::player_profile profile{};

  profile.coins = 1000;

  constexpr int reward = 123;
  Json::Value response;
  response["coins"] = reward;

  http.next_response["client/game/consume-reward"] = { 200, response };

  bool done_called = false;
  bim::app::consume_game_reward_job job(analytics, headers, profile);
  job.connect_to_done(
      [&](std::int64_t coins)
        {
          EXPECT_FALSE(done_called);
          EXPECT_EQ(reward, coins);
          done_called = true;
        });
  job.connect_to_error(
      []()
        {
          EXPECT_FALSE(true);
        });

  job.start(1010);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  EXPECT_EQ(1000 + reward, profile.coins);
}

TEST(bim_app_consume_game_reward_job, error)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  bim::business::request_headers headers;
  bim::app::player_profile profile{};

  profile.coins = 1000;

  http.next_response["client/game/consume-reward"] = { 422,
                                                       Json::objectValue };

  bool error_called = false;
  bim::app::consume_game_reward_job job(analytics, headers, profile);
  job.connect_to_done(
      [&](std::int64_t coins)
        {
          EXPECT_FALSE(true);
        });
  job.connect_to_error(
      [&]()
        {
          error_called = true;
        });

  job.start(10);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(error_called);
  EXPECT_EQ(1000, profile.coins);
}
