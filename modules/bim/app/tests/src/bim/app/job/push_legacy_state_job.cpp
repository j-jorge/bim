// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/push_legacy_state_job.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/preference/legacy.hpp>

#include <bim/business/request_headers.hpp>

#include <bim/app/tests/http_service.hpp>
#include <bim/app/tests/scheduler.hpp>

#include <bim/game/feature_flags.hpp>

#include <iscool/preferences/local_preferences.hpp>

#include <gtest/gtest.h>

class bim_app_push_legacy_state_job : public testing::Test
{
public:
  void start_job(iscool::preferences::local_preferences& preferences);

protected:
  bim::app::tests::scheduler m_scheduler;
  bim::app::tests::http_service m_http;
  bim::app::analytics_service m_analytics;
  const bim::business::request_headers m_headers;
};

void bim_app_push_legacy_state_job::start_job(
    iscool::preferences::local_preferences& preferences)
{
  Json::Value response;
  response["transfer_state"] = 1;

  m_http.next_response["client/transfer-legacy-inventory"] = { 200, response };

  bool done_called = false;
  bim::app::push_legacy_state_job job(m_analytics, m_headers, preferences);

  job.connect_to_done(
      [&]()
        {
          EXPECT_TRUE(bim::app::legacy_inventory_pushed(preferences));
          done_called = true;
        });

  job.start();

  m_scheduler.tick(std::chrono::seconds(1));

  EXPECT_TRUE(done_called);
}

TEST_F(bim_app_push_legacy_state_job, empty_everything)
{
  iscool::preferences::local_preferences preferences({});

  start_job(preferences);
}

TEST_F(bim_app_push_legacy_state_job, available_empty_slots)
{
  iscool::preferences::local_preferences preferences({});

  preferences.set_value("feature_flags.slot_0.available", true);
  preferences.set_value("feature_flags.slot_1.available", true);

  start_job(preferences);
}

TEST_F(bim_app_push_legacy_state_job, full_inventory)
{
  iscool::preferences::local_preferences preferences({});

  preferences.set_value("coins", (std::int64_t)24);

  preferences.set_value("feature_flags.slot_0.available", true);
  preferences.set_value("feature_flags.slot_1.available", false);

  preferences.set_value("game_features",
                        std::int64_t(bim::game::feature_flags::falling_blocks
                                     | bim::game::feature_flags::fences));

  preferences.set_value(
      "feature_flags.slot_0",
      (std::int64_t)bim::game::feature_flags::falling_blocks);

  preferences.set_value("arena_stats.game_count", (std::int64_t)10);
  preferences.set_value("arena_stats.victory_count", (std::int64_t)20);
  preferences.set_value("arena_stats.defeat_count", (std::int64_t)30);

  start_job(preferences);
}
