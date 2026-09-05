// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/validate_purchase_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business/purchase_validation_status.hpp>
#include <bim/app/business/validate_purchase.hpp>

#include <bim/business/request_headers.hpp>

#include <gtest/gtest.h>

TEST(bim_app_validate_purchase_job, success_ok)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  bim::business::request_headers headers;
  bim::app::player_profile profile{};

  Json::Value response;
  response["coins"] = 213;
  response["status"] = 0;

  http.next_response["client/billing/validate-purchase"] = { 200, response };

  bool done_called = false;
  bim::app::validate_purchase_job job(analytics, headers, profile);
  job.connect_to_done(
      [&](bim::app::purchase_validation_status status)
        {
          EXPECT_FALSE(done_called);
          EXPECT_EQ(bim::app::purchase_validation_status::ok, status);
          EXPECT_EQ(213, profile.coins);

          done_called = true;
        });

  job.start("token");

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
}
