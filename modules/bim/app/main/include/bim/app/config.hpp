// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

#include <bim/bit_map.hpp>

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace Json
{
  class Value;
}

namespace bim::app
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

    bim::bit_map<bim::game::feature_flags, std::int16_t> game_feature_price;

    std::vector<std::string> shop_products;
    std::vector<int> shop_product_coins;

    std::string discord_url;
  };

  std::optional<config> load_config(const Json::Value& json);
}
