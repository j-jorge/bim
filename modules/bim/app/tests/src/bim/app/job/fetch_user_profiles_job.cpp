// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/fetch_user_profiles_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/user_profile.hpp>

#include <bim/business/request_headers.hpp>

#include <gtest/gtest.h>

TEST(bim_app_fetch_user_profiles_job, success)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  const bim::business::request_headers headers;

  Json::Value response;
  response["profiles"][0]["user_id"] = 11;
  response["profiles"][0]["nickname"] = "foo";
  response["profiles"][1]["user_id"] = 22;
  response["profiles"][1]["nickname"] = "bar";

  http.next_response["client/users/profiles"] = { 200, response };

  int expected_step = 0;
  bim::app::fetch_user_profiles_job job(analytics, headers);
  job.connect_to_done(
      [&](std::span<bim::app::user_profile> profiles)
        {
          if (expected_step == 0)
            {
              ASSERT_EQ(2, profiles.size());

              EXPECT_EQ(11, profiles[0].user_id);
              EXPECT_EQ("foo", profiles[0].nickname);

              EXPECT_EQ(22, profiles[1].user_id);
              EXPECT_EQ("bar", profiles[1].nickname);

              Json::Value response;
              response["profiles"][0]["user_id"] = 33;
              response["profiles"][0]["nickname"] = "baz";

              http.next_response["client/users/profiles"] = { 200, response };
            }
          else if (expected_step == 1)
            {
              ASSERT_EQ(1, profiles.size());

              EXPECT_EQ(33, profiles[0].user_id);
              EXPECT_EQ("baz", profiles[0].nickname);
            }
          else
            EXPECT_FALSE(true);

          ++expected_step;
        });

  const bim::net::user_id batch_0[2] = { 1, 2 };
  const bim::net::user_id batch_1[1] = { 3 };

  job.start(batch_0);
  job.start(batch_1);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_EQ(2, expected_step);
}
