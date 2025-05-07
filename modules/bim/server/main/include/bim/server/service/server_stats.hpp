#pragma once
#include "bits/chrono.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iscool/schedule/manual_scheduler.hpp>
#include <mutex>

namespace bim::server
{
  using hour_time =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::hours>;

  using day_time =
      std::chrono::time_point<std::chrono::system_clock, std::chrono::days>;

  class server_stats
  {
  public:
    explicit server_stats(iscool::schedule::manual_scheduler& scheduler);

    // Session tracking
    void record_session_connected();
    void record_session_disconnected();

    // Game tracking
    void record_game_start(uint8_t player_count);
    void record_game_end(uint8_t player_count);

  private:
    // scheduler for data dumps & log rotation
    iscool::schedule::manual_scheduler& m_scheduler;

    // Logging control
    void schedule_next_hourly_dump();
    void schedule_daily_rotation();
    void rotate_log();
    void dump_hourly_stats();

    // Tracked Stats counters
    std::atomic<int> m_active_sessions{ 0 };
    std::atomic<int> m_players_in_games{ 0 };
    std::atomic<int> m_current_games{ 0 };
    std::atomic<int> m_games_this_hour{ 0 };

    // Logging state
    std::mutex m_log_mutex;
    std::ofstream m_log_file;

    void write_header();
    void write_current_stats();
  };
}
