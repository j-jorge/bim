// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/statistics_service.hpp>

#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>
#include <bim/server/tests/statistics_log.hpp>

#include <chrono>

#include <gtest/gtest.h>

TEST(statistics_service, records_session_activity_to_file)
{
  const bim::server::tests::statistics_log statistics_log;

  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_statistics_log = true;
  config.statistics_log_file = statistics_log.log_file();
  config.statistics_dump_delay = std::chrono::seconds(0);

  bim::server::tests::fake_scheduler scheduler;
  {
    bim::server::statistics_service stats(config);

    // Record some session activity
    stats.record_session_connected();
    stats.record_session_connected();

    // Advance time to trigger the delayed dump
    scheduler.tick(std::chrono::seconds(2));
  }

  const std::vector<bim::server::tests::statistics_log_line> log =
      statistics_log.read_log_file();

  ASSERT_EQ(1, log.size());
  EXPECT_EQ(2, log[0].active_sessions);
  EXPECT_EQ(0, log[0].players_in_games);
  EXPECT_EQ(0, log[0].games);
}

TEST(statistics_service, tracks_game_statistics_correctly)
{
  const bim::server::tests::statistics_log statistics_log;

  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_statistics_log = true;
  config.statistics_log_file = statistics_log.log_file();
  config.statistics_dump_delay = std::chrono::seconds(0);

  bim::server::tests::fake_scheduler scheduler;
  bim::server::statistics_service stats(config);

  // Start a game with 4 players
  stats.record_game_start(4);
  scheduler.tick(std::chrono::seconds(2));

  {
    const std::vector<bim::server::tests::statistics_log_line> log =
        statistics_log.read_log_file();

    ASSERT_EQ(1, log.size());
    EXPECT_EQ(0, log[0].active_sessions);
    EXPECT_EQ(4, log[0].players_in_games);
    EXPECT_EQ(1, log[0].games);
  }

  // End the game
  stats.record_game_end(4);
  scheduler.tick(std::chrono::seconds(2));

  const std::vector<bim::server::tests::statistics_log_line> log =
      statistics_log.read_log_file();

  ASSERT_EQ(2, log.size());
  EXPECT_EQ(0, log[0].active_sessions);
  EXPECT_EQ(4, log[0].players_in_games);
  EXPECT_EQ(1, log[0].games);

  EXPECT_EQ(0, log[1].active_sessions);
  EXPECT_EQ(0, log[1].players_in_games);
  EXPECT_EQ(0, log[1].games);
}

TEST(statistics_service, disabled_stats_recording_logs_nothing)
{
  const bim::server::tests::statistics_log statistics_log;

  bim::server::config config = bim::server::tests::new_test_config();
  config.statistics_log_file = statistics_log.log_file();
  config.statistics_dump_delay = std::chrono::seconds(0);

  bim::server::tests::fake_scheduler scheduler;
  bim::server::statistics_service stats(config);

  // Record some activity
  stats.record_session_connected();
  stats.record_game_start(2);

  scheduler.tick(std::chrono::seconds(2));

  const std::vector<bim::server::tests::statistics_log_line> log =
      statistics_log.read_log_file();

  EXPECT_EQ(0, log.size());
}
