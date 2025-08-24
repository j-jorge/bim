// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/config.hpp>

#include <iscool/signals/scoped_connection.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>

namespace bim::server
{
  class statistics_service
  {
  public:
    explicit statistics_service(const config& config);
    ~statistics_service();

    statistics_service(const statistics_service&) = delete;
    statistics_service& operator=(const statistics_service&) = delete;

    void record_session_connected();
    void record_session_disconnected();

    void record_game_start(uint8_t player_count);
    void record_game_end(uint8_t player_count);

  private:
    void conditional_dump();
    void schedule_file_dump();
    void dump_stats_to_file();

  private:
    int m_active_sessions;
    int m_players_in_games;
    int m_games;

    bool m_enabled;

    iscool::signals::scoped_connection m_file_dump_connection;

    std::chrono::seconds m_file_dump_delay;

    std::FILE* m_log_file;
  };
}
