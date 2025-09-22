// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/statistics_service.hpp>

#include <bim/server/config.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

#include <cassert>
#include <chrono>
#include <ctime>

bim::server::statistics_service::rolling_measure::rolling_measure()
  : last_hour(std::chrono::minutes(1), std::chrono::hours(1))
  , last_day(std::chrono::hours(1), std::chrono::days(1))
  , last_month(std::chrono::days(1), std::chrono::months(1))
{}

void bim::server::statistics_service::rolling_measure::tick(
    std::chrono::nanoseconds now)
{
  last_hour.push(now, 0);
  last_day.push(now, 0);
  last_month.push(now, 0);
}

void bim::server::statistics_service::rolling_measure::add(
    std::chrono::nanoseconds now, std::uint32_t value)
{
  last_hour.push(now, value);
  last_day.push(now, value);
  last_month.push(now, value);
}

bim::server::statistics_service::statistics_service(const config& config)
  : m_network_bytes_in(0)
  , m_network_bytes_out(0)
  , m_active_sessions_instant(0)
  , m_players_in_games_instant(0)
  , m_games_instant(0)
  , m_enable_file_dump(config.enable_statistics_log)
  , m_enable_rolling_statistics(config.enable_rolling_statistics)
  , m_file_dump_delay(config.statistics_dump_delay)
  , m_log_file(std::fopen(config.statistics_log_file.c_str(), "a"))
{
  if (m_enable_rolling_statistics)
    schedule_statistics_tick();

  if (m_enable_file_dump)
    {
      if (m_log_file)
        ic_log(iscool::log::nature::info(), "statistics_service",
               "Dumping statistics in '{}'.", config.statistics_log_file);
      else
        ic_log(iscool::log::nature::error(), "statistics_service",
               "Could not open '{}'.", config.statistics_log_file);
    }
}

bim::server::statistics_service::~statistics_service()
{
  if (m_log_file)
    std::fclose(m_log_file);
}

std::uint32_t bim::server::statistics_service::games_now() const
{
  return m_games_instant;
}

std::uint32_t bim::server::statistics_service::games_last_hour() const
{
  return m_games.last_hour.total();
}

std::uint32_t bim::server::statistics_service::games_last_day() const
{
  return m_games.last_day.total();
}

std::uint32_t bim::server::statistics_service::games_last_month() const
{
  return m_games.last_month.total();
}

std::uint32_t bim::server::statistics_service::sessions_now() const
{
  return m_active_sessions_instant;
}

std::uint32_t bim::server::statistics_service::sessions_last_hour() const
{
  return m_active_sessions.last_hour.total();
}

std::uint32_t bim::server::statistics_service::sessions_last_day() const
{
  return m_active_sessions.last_day.total();
}

std::uint32_t bim::server::statistics_service::sessions_last_month() const
{
  return m_active_sessions.last_month.total();
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
  ++m_active_sessions_instant;

  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();
  m_active_sessions.add(now, 1);

  schedule_file_dump();
}

void bim::server::statistics_service::record_session_disconnected(
    std::uint32_t count)
{
  if (count == 0)
    return;

  assert(count <= m_active_sessions_instant);

  m_active_sessions_instant -= count;

  schedule_file_dump();
}

void bim::server::statistics_service::record_game_start(
    std::uint8_t player_count)
{
  ++m_games_instant;
  m_players_in_games_instant += player_count;

  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  m_games.add(now, 1);
  m_players_in_games.add(now, player_count);

  schedule_file_dump();
}

void bim::server::statistics_service::record_game_end(
    std::uint8_t player_count)
{
  assert(m_games_instant > 0);
  assert(player_count <= m_players_in_games_instant);

  --m_games_instant;
  m_players_in_games_instant -= player_count;

  schedule_file_dump();
}

void bim::server::statistics_service::schedule_file_dump()
{
  if (!m_enable_file_dump || m_file_dump_connection.connected())
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

  std::fprintf(
      m_log_file, "%s %u %u %u %lu %lu %u %u %u %u %u %u %u %u %u\n",
      date_string, m_active_sessions_instant, m_players_in_games_instant,
      m_games_instant, m_network_bytes_in, m_network_bytes_out,
      m_active_sessions.last_hour.total(), m_active_sessions.last_day.total(),
      m_active_sessions.last_month.total(),
      m_players_in_games.last_hour.total(),
      m_players_in_games.last_day.total(),
      m_players_in_games.last_month.total(), m_games.last_hour.total(),
      m_games.last_day.total(), m_games.last_month.total());
  std::fflush(m_log_file);
}

void bim::server::statistics_service::schedule_statistics_tick()
{
  if (!m_enable_rolling_statistics || m_statistics_tick_connection.connected())
    return;

  m_statistics_tick_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        const std::chrono::seconds now =
            iscool::time::now<std::chrono::seconds>();

        m_active_sessions.tick(now);
        m_players_in_games.tick(now);
        m_games.tick(now);

        schedule_statistics_tick();
      },
      std::chrono::minutes(1));
}
