// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/config.hpp>

bim::server::config::config()
  : port(65535)
  , random_seed(0)
  , authentication_clean_up_interval(std::chrono::minutes(3))
  , matchmaking_clean_up_interval(std::chrono::minutes(3))
  , random_game_auto_start_delay(std::chrono::seconds(20))
  , game_service_clean_up_interval(std::chrono::minutes(3))
  , game_service_disconnection_lateness_threshold_in_ticks(75)
  , game_service_disconnection_earliness_threshold_in_ticks(75)
  , game_service_disconnection_inactivity_delay(std::chrono::seconds(10))
  , enable_contest_timeline_recording(false)
  , geolocation_clean_up_interval(std::chrono::days(7))
  , geolocation_update_interval(std::chrono::days(7))
  , enable_geolocation(false)
  , enable_statistics_log(false)
  , statistics_dump_delay(std::chrono::seconds(60))
{}
