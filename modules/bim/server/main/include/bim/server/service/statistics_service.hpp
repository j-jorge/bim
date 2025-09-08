// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/config.hpp>

#include <iscool/schedule/scoped_connection.hpp>

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

    void network_traffic(std::uint64_t bytes_in, std::uint64_t bytes_out);

    void record_session_connected();
    void record_session_disconnected(std::uint64_t count);

    void record_game_start(std::uint8_t player_count);
    void record_game_end(std::uint8_t player_count);

  private:
    void schedule_file_dump();
    void dump_stats_to_file();

  private:
    std::uint64_t m_network_bytes_in;
    std::uint64_t m_network_bytes_out;

    std::uint64_t m_active_sessions;
    std::uint64_t m_players_in_games;
    std::uint64_t m_games;

    bool m_enabled;

    iscool::schedule::scoped_connection m_file_dump_connection;

    std::chrono::seconds m_file_dump_delay;

    std::FILE* m_log_file;
  };
}
