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

  http.next_response["client/account/update-nickname"] = { 200,
                                                           Json::objectValue };

  bool done_called = false;

  bim::app::update_nickname_job job(analytics, headers, profile);
  job.connect_to_done(
      [&]()
        {
          EXPECT_FALSE(done_called);
          EXPECT_EQ("bar", profile.nickname);
          done_called = true;
        });

  job.start("bar");

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  EXPECT_EQ("bar", profile.nickname);
}
