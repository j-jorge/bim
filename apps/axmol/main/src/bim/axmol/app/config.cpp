// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>
#include <bim/version.hpp>

#include <iscool/json/cast_int.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/cast_uint.hpp>
#include <iscool/json/is_member.hpp>
#include <iscool/json/is_of_type_int16.hpp>
#include <iscool/json/is_of_type_string.hpp>
#include <iscool/json/is_of_type_uint.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/warning.hpp>

#include <type_traits>

static bool parse_server_list(bim::axmol::app::config& result,
                              const Json::Value& servers)
{
  if (!servers.isArray())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Server list is not an array.");
      return false;
    }

  std::string app_version_host;
  std::string protocol_version_host;
  std::string default_host;

  for (const Json::Value& server : servers)
    {
      if (!server.isObject() || !iscool::json::is_member("h", server))
        continue;

      const Json::Value& host = server["h"];

      if (!iscool::json::is_of_type<std::string>(host))
        continue;

      std::string host_str = iscool::json::cast<std::string>(host);

      const bool has_app_version = iscool::json::is_member("v", server);
      const bool has_protocol_version = iscool::json::is_member("p", server);

      if (has_app_version)
        {
          const Json::Value& version = server["v"];

          if (iscool::json::is_of_type<unsigned int>(version))
            {
              const int v = iscool::json::cast<unsigned int>(version);
              if (v == bim::version_major)
                app_version_host = host_str;
            }
        }

      if (has_protocol_version)
        {
          const Json::Value& version = server["p"];

          if (iscool::json::is_of_type<bim::net::version>(version))
            {
              const bim::net::version v =
                  iscool::json::cast<bim::net::version>(version);
              if (v == bim::net::protocol_version)
                protocol_version_host = std::move(host_str);
            }
        }

      if (!has_app_version && !has_protocol_version)
        default_host = std::move(host_str);
    }

  if (!app_version_host.empty())
    result.game_server = std::move(app_version_host);
  else if (!protocol_version_host.empty())
    result.game_server = std::move(protocol_version_host);
  else if (!default_host.empty())
    result.game_server = std::move(default_host);
  else
    {
      ic_log(iscool::log::nature::warning(), "config",
             "No game server configured.");
      return false;
    }

  return true;
}

static bool parse_shop_products(bim::axmol::app::config& result,
                                const Json::Value& products)
{
  if (products.isNull())
    return true;

  if (!products.isArray())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Shop products is not an array.");
      return false;
    }

  const std::size_t product_count = products.size();

  std::vector<std::string> shop_products;
  shop_products.reserve(product_count);

  std::vector<int> shop_product_coins;
  shop_product_coins.reserve(product_count);

  for (const Json::Value& item : products)
    {
      if (!item.isObject() || !iscool::json::is_member("product-id", item)
          || !iscool::json::is_member("coins", item))
        continue;

      const Json::Value& id = item["product-id"];
      const Json::Value& coins = item["coins"];

      if (!iscool::json::is_of_type<std::string>(id)
          || !iscool::json::is_of_type<int>(coins))
        continue;

      shop_products.push_back(iscool::json::cast<std::string>(id));
      shop_product_coins.push_back(iscool::json::cast<int>(coins));
    }

  result.shop_products.swap(shop_products);
  result.shop_product_coins.swap(shop_product_coins);

  return true;
}

template <typename T>
static bool read_value(T& r, const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    return true;

  const Json::Value& v = json[n];

  if (!iscool::json::is_of_type<T>(v))
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Incorrect type for '%s'.", n);
      return false;
    }

  r = iscool::json::cast<T>(v);

  return true;
}

static bool parse_game_feature_prices(bim::axmol::app::config& result,
                                      const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    return true;

  const Json::Value& prices = json[n];

  if (!prices.isObject())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Game feature prices is not an object.");
      return false;
    }

  if (!read_value(
          result.game_feature_price[bim::game::feature_flags::falling_blocks],
          prices, "fb"))
    return false;

  if (!read_value(result.game_feature_price[bim::game::feature_flags::shield],
                  prices, "s"))
    return false;

  if (!read_value(
          result.game_feature_price[bim::game::feature_flags::invisibility],
          prices, "i"))
    return false;

  if (!read_value(
          result.game_feature_price[bim::game::feature_flags::fog_of_war],
          prices, "fow"))
    return false;

  return true;
}

bim::axmol::app::config::config()
  : most_recent_version(bim::version_major)
  , game_server("bim.jorge.st:"
                + std::to_string(20000 + bim::version_major * 100
                                 + bim::net::protocol_version))
  , remote_config_update_interval(std::chrono::hours(1))
  , version_update_interval(std::chrono::days(1))
  , coins_per_victory(50)
  , coins_per_defeat(10)
  , coins_per_draw(10)
{
  game_feature_price[bim::game::feature_flags::falling_blocks] = 50;
  game_feature_price[bim::game::feature_flags::shield] = 250;
  game_feature_price[bim::game::feature_flags::invisibility] = 250;
  game_feature_price[bim::game::feature_flags::fog_of_war] = 1000;
}

std::optional<bim::axmol::app::config>
bim::axmol::app::load_config(const Json::Value& json)
{
  if (!json.isObject())
    return std::nullopt;

  config result;

  result.most_recent_version =
      iscool::json::cast<unsigned int>(json["mrv"], bim::version_major);

  const auto read_hours = [&json](std::chrono::hours& r, const char* n) -> bool
  {
    std::uint64_t h = r.count();

    if (read_value(h, json, n))
      {
        r = std::chrono::hours(h);
        return true;
      }

    return false;
  };

  if (!read_hours(result.remote_config_update_interval, "rcui"))
    return std::nullopt;

  if (!read_hours(result.version_update_interval, "vui"))
    return std::nullopt;

  if (!parse_server_list(result, json["gs"]))
    return std::nullopt;

  if (!read_value(result.coins_per_victory, json, "cpv"))
    return std::nullopt;

  if (!read_value(result.coins_per_defeat, json, "cpde"))
    return std::nullopt;

  if (!read_value(result.coins_per_draw, json, "cpdr"))
    return std::nullopt;

  if (!parse_game_feature_prices(result, json, "gfp"))
    return std::nullopt;

  if (!parse_shop_products(result, json["shop"]))
    return std::nullopt;

  return result;
}
