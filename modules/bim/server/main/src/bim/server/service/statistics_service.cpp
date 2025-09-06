// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/statistics_service.hpp>

#include <bim/server/config.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

#include <chrono>
#include <ctime>

bim::server::statistics_service::statistics_service(const config& config)
  : m_network_bytes_in(0)
  , m_network_bytes_out(0)
  , m_active_sessions(0)
  , m_players_in_games(0)
  , m_games(0)
  , m_enabled(config.enable_statistics_log)
  , m_file_dump_delay(config.statistics_dump_delay)
  , m_log_file(std::fopen(config.statistics_log_file.c_str(), "a"))
{
  if (!m_enabled)
    return;

  if (m_log_file)
    {
      ic_log(iscool::log::nature::info(), "statistics_service",
             "Dumping statistics in '{}'.", config.statistics_log_file);
    }
  else
    ic_log(iscool::log::nature::error(), "statistics_service",
           "Could not open '{}'.", config.statistics_log_file);
}

bim::server::statistics_service::~statistics_service()
{
  if (m_log_file)
    std::fclose(m_log_file);
}

void bim::server::statistics_service::network_traffic(std::uint64_t bytes_in,
                                                      std::uint64_t bytes_out)
{
  m_network_bytes_in = bytes_in;
  m_network_bytes_out = bytes_out;

  schedule_file_dump();
}

void bim::server::statistics_service::record_session_connected()
{
  ++m_active_sessions;
  schedule_file_dump();
}

void bim::server::statistics_service::record_session_disconnected()
{
  --m_active_sessions;
  schedule_file_dump();
}

void bim::server::statistics_service::record_game_start(uint8_t player_count)
{
  ++m_games;
  m_players_in_games += player_count;

  schedule_file_dump();
}

void bim::server::statistics_service::record_game_end(uint8_t player_count)
{
  --m_games;
  m_players_in_games -= player_count;

  schedule_file_dump();
}

void bim::server::statistics_service::schedule_file_dump()
{
  if (!m_enabled || m_file_dump_connection.connected())
    return;

  m_file_dump_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        dump_stats_to_file();
      },
      m_file_dump_delay);
}

void bim::server::statistics_service::dump_stats_to_file()
{
  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  const std::time_t timer = std::chrono::system_clock::to_time_t(
      std::chrono::time_point<std::chrono::system_clock>(now));

  std::tm gmt;
  gmtime_r(&timer, &gmt);

  constexpr std::size_t date_capacity = 20;
  char date_string[date_capacity];

  if (std::strftime(date_string, date_capacity, "%Y-%m-%d %H:%M:%S", &gmt)
      == 0)
    {
      ic_log(iscool::log::nature::error(), "statistics_service",
             "Failed to format current date in {} bytes.", date_capacity);
      return;
    }

  std::fprintf(m_log_file, "%s %d %d %d %lu %lu\n", date_string,
               m_active_sessions, m_players_in_games, m_games,
               m_network_bytes_in, m_network_bytes_out);
  std::fflush(m_log_file);
}
