// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/config.hpp>
#include <bim/server/rolling_statistics.hpp>

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

    std::uint32_t games_now() const;
    std::uint32_t games_last_hour() const;
    std::uint32_t games_last_day() const;
    std::uint32_t games_last_month() const;

    std::uint32_t sessions_now() const;
    std::uint32_t sessions_last_hour() const;
    std::uint32_t sessions_last_day() const;
    std::uint32_t sessions_last_month() const;

    void network_traffic(std::uint64_t bytes_in, std::uint64_t bytes_out);

    void record_session_connected();
    void record_session_disconnected(std::uint32_t count);

    void record_game_start(std::uint8_t player_count);
    void record_game_end(std::uint8_t player_count);

  private:
    struct rolling_measure
    {
      rolling_measure();

      void tick(std::chrono::nanoseconds now);
      void add(std::chrono::nanoseconds now, std::uint32_t value);

      rolling_statistics last_hour;
      rolling_statistics last_day;
      rolling_statistics last_month;
    };

  private:
    void schedule_file_dump();
    void dump_stats_to_file();

    void schedule_statistics_tick();

  private:
    std::uint64_t m_network_bytes_in;
    std::uint64_t m_network_bytes_out;

    rolling_measure m_active_sessions;
    rolling_measure m_players_in_games;
    rolling_measure m_games;

    std::uint32_t m_active_sessions_instant;
    std::uint32_t m_players_in_games_instant;
    std::uint32_t m_games_instant;

    bool m_enable_file_dump;
    bool m_enable_rolling_statistics;

    iscool::schedule::scoped_connection m_file_dump_connection;
    iscool::schedule::scoped_connection m_statistics_tick_connection;

    std::chrono::seconds m_file_dump_delay;

    std::FILE* m_log_file;
  };
}
