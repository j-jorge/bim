#include <bim/server/service/server_stats.hpp>

namespace bim::server
{
  server_stats::server_stats(iscool::schedule::manual_scheduler& schedular)
    : m_scheduler(schedular)
  {
    schedule_next_hourly_dump();
    schedule_daily_rotation();
  }

  void server_stats::record_session_connected()
  {
    m_active_sessions++;
  }

  void server_stats::record_session_disconnected()
  {
    m_active_sessions--;
  }

  void server_stats::record_game_start(uint8_t player_count)
  {
    m_current_games++;
    m_players_in_games += player_count;
    m_games_this_hour++;
  }

  void server_stats::record_game_end(uint8_t player_count)
  {
    m_current_games--;
    m_players_in_games -= player_count;
  }

  void server_stats::dump_hourly_stats()
  {
    std::lock_guard lock(m_log_mutex);
    write_current_stats();
  }

  void server_stats::write_current_stats()
  {
    const std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();

    m_log_file << std::format("{:%T},", now) << m_active_sessions.load() << ","
               << m_players_in_games.load() << "," << m_current_games.load()
               << "," << m_games_this_hour.load() << "\n";

    // Reset hourly counter
    m_games_this_hour = 0;
  }

  void server_stats::write_header()
  {
    m_log_file << "Time, active_sessions , players_in_game, current_games, "
                  "games_this_hour"
               << "\n";
  }

  void server_stats::schedule_next_hourly_dump()
  {
    const std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    hour_time current_hour = std::chrono::floor<std::chrono::hours>(now);
    hour_time next_hour = current_hour + std::chrono::hours(1);

    std::chrono::system_clock::duration delay = next_hour - now;

    // Convert delay to nanoseconds (as expected by the scheduler)
    const std::chrono::nanoseconds delay_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(delay);

    // Schedule the next hourly dump
    m_scheduler.get_delayed_call_delegate()(
        [this]()
        {
          dump_hourly_stats();
          schedule_next_hourly_dump(); // Schedule the next data dump
        },
        delay_ns);
  }

  void server_stats::schedule_daily_rotation()
  {
    const std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    day_time current_day = std::chrono::floor<std::chrono::days>(now);
    day_time next_day = current_day + std::chrono::days(1);

    std::chrono::system_clock::duration delay = next_day - now;
    const std::chrono::nanoseconds delay_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(delay);

    m_scheduler.get_delayed_call_delegate()(
        [this]()
        {
          rotate_log();
          schedule_daily_rotation(); // Schedule the next rotation
        },
        delay_ns);
  }

  void server_stats::rotate_log()
  {
    std::lock_guard lock(m_log_mutex);
    if (m_log_file.is_open())
      m_log_file.close();

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream filename;
    filename << "stats_" << std::put_time(std::localtime(&t), "%Y-%m-%d")
             << ".log";

    m_log_file.open(filename.str(), std::ios::out | std::ios::app);
    write_header();
  }

}
