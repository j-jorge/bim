// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/app/job/assign_feature_slots_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/business/player_profile.hpp>

#include <bim/business/request_headers.hpp>

#include <bim/game/feature_flags.hpp>

#include <gtest/gtest.h>

TEST(bim_app_assign_feature_slots_job, success)
{
  bim::app::tests::scheduler scheduler;
  bim::app::tests::http_service http;

  bim::app::analytics_service analytics;
  bim::business::request_headers headers;
  bim::app::player_profile profile{};

  profile.slot_availability.fill(true);
  profile.coins = 1000;

  http.next_response["client/game-feature/assign-slots"] = {
    200, Json::objectValue
  };

  bool done_called = false;
  bim::app::assign_feature_slots_job job(analytics, headers, profile);
  job.connect_to_done(
      [&]()
        {
          http.next_response["client/game-feature/assign-slots"] = {
            200, Json::objectValue
          };

          done_called = true;
        });

  job.assign_slots({ std::nullopt, bim::game::feature_flags::shield });
  job.assign_slots({ bim::game::feature_flags::fog_of_war, std::nullopt });
  job.assign_slots({ bim::game::feature_flags::fog_of_war,
                     bim::game::feature_flags::invisibility });

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  done_called = false;
  EXPECT_EQ(bim::game::feature_flags{}, profile.slot_feature[0]);
  EXPECT_EQ(bim::game::feature_flags::shield, profile.slot_feature[1]);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  done_called = false;
  EXPECT_EQ(bim::game::feature_flags::fog_of_war, profile.slot_feature[0]);
  EXPECT_EQ(bim::game::feature_flags::invisibility, profile.slot_feature[1]);

  job.assign_slots({ bim::game::feature_flags::falling_blocks, std::nullopt });

  http.next_response["client/game-feature/clear-slot"] = { 200,
                                                           Json::objectValue };

  job.clear_slot(0);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  done_called = false;
  EXPECT_EQ(bim::game::feature_flags::falling_blocks, profile.slot_feature[0]);
  EXPECT_EQ(bim::game::feature_flags::invisibility, profile.slot_feature[1]);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
  done_called = false;
  EXPECT_EQ(bim::game::feature_flags{}, profile.slot_feature[0]);
  EXPECT_EQ(bim::game::feature_flags::invisibility, profile.slot_feature[1]);

  scheduler.tick(std::chrono::seconds(1));

  EXPECT_FALSE(done_called);
}
