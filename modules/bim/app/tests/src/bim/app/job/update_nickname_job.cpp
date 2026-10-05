// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/update_nickname_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/config.hpp>

#include <bim/business/request_headers.hpp>

#include <gtest/gtest.h>

TEST(bim_app_update_nickname_job, success)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  const bim::business::request_headers headers;
  bim::app::player_profile profile{};

  profile.nickname = "foo";

  Json::Value r;
  r["nickname_change_allowed_date"] = "1971-02-03T04:05:06.123456Z";

  http.next_response["client/account/update-nickname"] = { 200, r };

  const std::chrono::system_clock::time_point
      expected_nickname_change_allowed_date =
          std::chrono::sys_days(std::chrono::year_month_day(
              std::chrono::year(1971), std::chrono::February,
              std::chrono::day{ 3 }))
          + std::chrono::hours(4) + std::chrono::minutes(5)
          + std::chrono::seconds(6) + std::chrono::microseconds(123456);

  bool done_called = false;

  bim::app::update_nickname_job job(analytics, headers, profile);
  job.connect_to_done(
      [&]()
        {
          EXPECT_FALSE(done_called);
          EXPECT_EQ("bar", profile.nickname);
          EXPECT_EQ(expected_nickname_change_allowed_date,
                    profile.nickname_change_allowed_date);
          done_called = true;
        });

  job.start("bar");

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  EXPECT_EQ("bar", profile.nickname);
  EXPECT_EQ(expected_nickname_change_allowed_date,
            profile.nickname_change_allowed_date);
}
