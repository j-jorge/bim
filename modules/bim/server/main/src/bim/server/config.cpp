// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/config.hpp>

#include <random>

bim::server::config::config()
  : port(65535)
  , random_seed(0)
  , authentication_clean_up_interval(std::chrono::minutes(3))
  , matchmaking_clean_up_interval(std::chrono::minutes(3))
  , random_game_auto_start_delay(std::chrono::seconds(20))
  , game_service_clean_up_interval(std::chrono::minutes(3))
  , game_service_disconnection_lateness_threshold_in_ticks(60)
  , game_service_disconnection_earliness_threshold_in_ticks(60)
  , enable_contest_timeline_recording(false)
{}
