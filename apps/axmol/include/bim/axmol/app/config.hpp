// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace Json
{
  class Value;
}

namespace bim::axmol::app
{
  struct config
  {
    config();

    int most_recent_version;
    std::string game_server;
    std::chrono::hours remote_config_update_interval;
    std::chrono::hours version_update_interval;

    int coins_per_victory;
    int coins_per_defeat;
    int coins_per_draw;
  };

  std::optional<config> load_config(const Json::Value& json);
}
