// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/connection.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::server
{
  class server_stats
  {
  public:
    explicit server_stats(std::chrono::minutes file_dump_delay,
                          std::chrono::days log_rotation_interval,
                          bool skip_dumping = false);
    // Session tracking
    void record_session_connected();
    void record_session_disconnected();

    // Game tracking
    void record_game_start(uint8_t player_count);
    void record_game_end(uint8_t player_count);

  private:
    // Tracked Stats counters
    int m_active_sessions = 0;
    int m_players_in_games = 0;
    int m_current_games = 0;

    // Logging control
    bool skip_file_dumping; // for testing without file operations

    // scheduler for data dumps & log rotation
    iscool::signals::scoped_connection m_file_dump_connection;

    // Logging state
    std::chrono::year_month_day m_log_file_ymd;
    std::chrono::minutes m_file_dump_delay;
    std::chrono::days m_log_rotation_interval;

    std::ofstream m_log_file;

    void schedule_file_dump();
    void rotate_log();
    void write_header();
    void conditional_dump();
    void dump_stats_to_file();
  };
}
