// SPDX-License-Identifier: AGPL-3.0-only
#include "bits/chrono.h"
#include <bim/server/config.hpp>
#include <bim/server/service/server_stats.hpp>
#include <chrono>
#include <ctime>
#include <iscool/time/now.hpp>

#include <filesystem>

bim::server::server_stats::server_stats(
    std::chrono::minutes file_dump_delay,
    std::chrono::days log_rotation_interval, bool skip_dumping)
  : skip_file_dumping(skip_dumping)
  , m_file_dump_delay(file_dump_delay)
  , m_log_rotation_interval(log_rotation_interval)
{
  if (!skip_file_dumping)
    {
      // Initialise and open log file
      rotate_log();
      // Schedule an initial dump
      schedule_file_dump();
    }
}

void bim::server::server_stats::record_session_connected()
{
  m_active_sessions++;
  conditional_dump();
}

void bim::server::server_stats::record_session_disconnected()
{
  m_active_sessions--;
  conditional_dump();
}

void bim::server::server_stats::record_game_start(uint8_t player_count)
{
  m_current_games++;
  m_players_in_games += player_count;
  conditional_dump();
}

void bim::server::server_stats::record_game_end(uint8_t player_count)
{
  m_current_games--;
  m_players_in_games -= player_count;
  conditional_dump();
}

void bim::server::server_stats::dump_stats_to_file()
{
  const std::chrono::system_clock::time_point now =
      std::chrono::system_clock::now();

  // Format time for logging
  const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm* gmt = std::gmtime(&now_time_t);
  std::ostringstream oss;
  oss << std::put_time(gmt, "%Y-%m-%d %H:%M:%S");

  const std::chrono::sys_days current_date =
      std::chrono::floor<std::chrono::days>(now);

  const std::chrono::sys_days log_file_start_date_as_sys_days =
      std::chrono::sys_days{ m_log_file_ymd };

  const std::chrono::sys_days next_rotation_date =
      log_file_start_date_as_sys_days + m_log_rotation_interval;

  if (current_date >= next_rotation_date)
    {
      rotate_log();
    }

  m_log_file << oss.str() << " " << " " << m_active_sessions << " "
             << m_players_in_games << " " << m_current_games << "\n";
  m_log_file.flush();
}

void bim::server::server_stats::write_header()
{
  m_log_file << "Time active_sessions players_in_game current_games" << "\n";
}

void bim::server::server_stats::rotate_log()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

  m_log_file_ymd = std::chrono::floor<std::chrono::days>(now);

  if (m_log_file.is_open())
    m_log_file.close();

  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream filename_stream;
  filename_stream << "stats_" << std::put_time(std::gmtime(&t), "%Y-%m-%d")
                  << ".log";
  const std::string filename = filename_stream.str();

  bool file_already_exists = std::filesystem::exists(filename);

  m_log_file.open(filename, std::ios::out | std::ios::app);

  if (!file_already_exists)
    {
      write_header();
    }
}

void bim::server::server_stats::conditional_dump()
{
  if (!skip_file_dumping)
    schedule_file_dump();
}

void bim::server::server_stats::schedule_file_dump()
{
  if (!m_file_dump_connection.connected())
    m_file_dump_connection = iscool::schedule::delayed_call(
        [this]() -> void
        {
          dump_stats_to_file();
          if (m_file_dump_connection.connected())
            {
              m_file_dump_connection.disconnect();
            }
        },
        m_file_dump_delay);
}
