#include <bim/server/service/server_stats.hpp>
#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

class ServerStatsTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Create a unique test directory for each test
    test_dir = std::filesystem::temp_directory_path()
               / ("server_stats_test_" + std::to_string(std::time(nullptr)));
    std::filesystem::create_directories(test_dir);
  }

  void TearDown() override
  {
    // Clean up test files
    if (std::filesystem::exists(test_dir))
      {
        std::filesystem::remove_all(test_dir);
      }
  }

  // Helper function to read file contents
  std::string read_file_contents(const std::filesystem::path& file_path)
  {
    std::ifstream file(file_path);
    if (!file.is_open())
      {
        return "";
      }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  // Helper function to count lines in a file
  int count_file_lines(const std::filesystem::path& file_path)
  {
    std::ifstream file(file_path);
    if (!file.is_open())
      {
        return 0;
      }
    int line_count = 0;
    std::string line;
    while (std::getline(file, line))
      {
        line_count++;
      }
    return line_count;
  }

  std::filesystem::path test_dir;
};

TEST_F(ServerStatsTest, creates_log_file_on_construction)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_server_stats_recording = true;
  config.server_stats_folder = test_dir;

  bim::server::tests::fake_scheduler scheduler;

  // Create server_stats - should create initial log file
  bim::server::server_stats stats(config);
  stats.flush_for_testing();

  // Check that a log file was created (should be named with current date)
  bool log_file_exists = false;
  for (const auto& entry : std::filesystem::directory_iterator(test_dir))
    {
      if (entry.path().extension() == ".log")
        {
          log_file_exists = true;

          // Verify header was written
          std::string contents = read_file_contents(entry.path());
          EXPECT_TRUE(contents.find("YYYY-MM-DD HH:MM:SS active_sessions")
                      != std::string::npos);
          break;
        }
    }

  EXPECT_TRUE(log_file_exists);
}

TEST_F(ServerStatsTest, records_session_activity_to_file)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_server_stats_recording = true;
  config.server_stats_folder = test_dir;
  config.stats_dump_delay = std::chrono::seconds(1); // Short delay for testing

  bim::server::tests::fake_scheduler scheduler;
  bim::server::server_stats stats(config);

  // Record some session activity
  stats.record_session_connected();
  stats.record_session_connected();

  // Advance time to trigger the delayed dump
  scheduler.tick(std::chrono::seconds(2));
  stats.flush_for_testing();

  // Find the log file and check its contents
  std::filesystem::path log_file;
  for (const auto& entry : std::filesystem::directory_iterator(test_dir))
    {
      if (entry.path().extension() == ".log")
        {
          log_file = entry.path();
          break;
        }
    }

  ASSERT_FALSE(log_file.empty());

  std::string contents = read_file_contents(log_file);

  // Should have header + at least one data line
  EXPECT_GT(count_file_lines(log_file), 1);

  // Check that the data line contains expected session count (2 active
  // sessions)
  EXPECT_TRUE(contents.find(" 2 ") != std::string::npos); // 2 active sessions
}

TEST_F(ServerStatsTest, rotates_log_after_time_interval)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_server_stats_recording = true;
  config.server_stats_folder = test_dir;
  config.stats_log_rotation_interval = std::chrono::days(1); // Rotate daily
  config.stats_dump_delay = std::chrono::seconds(1);

  bim::server::tests::fake_scheduler scheduler;
  bim::server::server_stats stats(config);

  // Record activity on day 1
  stats.record_session_connected();
  scheduler.tick(std::chrono::seconds(2)); // Let dump happen

  // Advance time by more than rotation interval (25 hours)
  scheduler.tick(std::chrono::hours(25));

  // Record activity on day 2 (should trigger rotation)
  stats.record_game_start(4);
  scheduler.tick(std::chrono::seconds(2));

  // Count log files - should have 2 files now
  int log_file_count = 0;
  for (const auto& entry : std::filesystem::directory_iterator(test_dir))
    {
      if (entry.path().extension() == ".log")
        {
          log_file_count++;
        }
    }

  EXPECT_EQ(log_file_count, 2) << "Expected 2 log files after rotation";
}

TEST_F(ServerStatsTest, tracks_game_statistics_correctly)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_server_stats_recording = true;
  config.server_stats_folder = test_dir;
  config.stats_dump_delay = std::chrono::seconds(1);

  bim::server::tests::fake_scheduler scheduler;
  bim::server::server_stats stats(config);

  // Start a game with 4 players
  stats.record_game_start(4);
  scheduler.tick(std::chrono::seconds(2));
  stats.flush_for_testing();

  // Find and read the log file
  std::filesystem::path log_file;
  for (const auto& entry : std::filesystem::directory_iterator(test_dir))
    {
      if (entry.path().extension() == ".log")
        {
          log_file = entry.path();
          break;
        }
    }

  std::string contents = read_file_contents(log_file);

  // Should show 4 players in games and 1 current game
  // Format: "YYYY-MM-DD HH:MM:SS active_sessions players_in_game
  // current_games"
  EXPECT_TRUE(contents.find(" 0 4 1") != std::string::npos)
      << "Expected '0 4 1' (0 sessions, 4 players in games, 1 current game)";

  // End the game
  stats.record_game_end(4);
  scheduler.tick(std::chrono::seconds(2));
  stats.flush_for_testing();

  // Read updated contents
  contents = read_file_contents(log_file);

  // Should now show 0 players and 0 games
  EXPECT_TRUE(contents.find(" 0 0 0") != std::string::npos)
      << "Expected '0 0 0' after game ended";
}

TEST_F(ServerStatsTest, disabled_stats_recording_creates_no_files)
{
  bim::server::config config = bim::server::tests::new_test_config();
  // config.enable_server_stats_recording = false; // Disabled by default
  config.server_stats_folder = test_dir;

  bim::server::tests::fake_scheduler scheduler;
  bim::server::server_stats stats(config);

  // Record some activity
  stats.record_session_connected();
  stats.record_game_start(2);

  scheduler.tick(std::chrono::seconds(62));
  stats.flush_for_testing();

  // Should be no log files created
  int log_file_count = 0;
  for (const auto& entry : std::filesystem::directory_iterator(test_dir))
    {
      if (entry.path().extension() == ".log")
        {
          log_file_count++;
        }
    }

  EXPECT_EQ(log_file_count, 0)
      << "No log files should be created when stats recording is disabled";
}
