// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/game/feature_flags.hpp>
#include <bim/game/feature_flags_string.hpp>

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
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/warning.hpp>

#include <type_traits>

template <typename T>
static bool read_value(T& r, const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    return true;

  const Json::Value& v = json[n];

  if (!iscool::json::is_of_type<T>(v))
    {
      ic_log(iscool::log::nature::error(), "config",
             "Incorrect type for '%s'.", n);
      return false;
    }

  r = iscool::json::cast<T>(v);

  return true;
}

static bool read_hours(std::chrono::hours& r, const Json::Value& json,
                       const char* n)
{
  std::uint64_t h = r.count();

  if (read_value(h, json, n))
    {
      r = std::chrono::hours(h);
      return true;
    }

  return false;
};

static bool parse_misc(bim::app::config& result, const Json::Value& json,
                       const char* n)
{
  if (!iscool::json::is_member(n, json))
    {
      ic_log(iscool::log::nature::error(), "config",
             "Missing misc entry in config.");
      return false;
    }

  const Json::Value& misc = json[n];

  if (!misc.isObject())
    {
      ic_log(iscool::log::nature::error(), "config", "Misc is not an object.");
      return false;
    }

  if (!read_value(result.most_recent_version, misc, "most_recent_version"))
    return false;

  if (!read_hours(result.version_update_interval, misc,
                  "version_update_interval"))
    return false;

  return true;
}

static bool parse_server_list(bim::app::config& result,
                              const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    {
      ic_log(iscool::log::nature::error(), "config",
             "Missing server list entry in config.");
      return false;
    }

  const Json::Value& hosts = json[n];

  if (!hosts.isArray())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Server list is not an array.");
      return false;
    }

  if (hosts.empty())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "No game server in client config.");
      return true;
    }

  result.game_server = iscool::json::cast<std::string>(hosts[0]);

  return true;
}

static bool parse_game_features(bim::app::config& result,
                                const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    {
      ic_log(iscool::log::nature::error(), "config",
             "Missing game features in config.");
      return false;
    }

  const Json::Value& entries = json[n];

  if (!entries.isArray())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Game features is not an array.");
      return false;
    }

  for (Json::Value::ArrayIndex i = 0, count = entries.size(); i != count; ++i)
    {
      const Json::Value& entry = entries[i];

      if (!entry.isObject())
        {
          ic_log(iscool::log::nature::error(), "config",
                 "Game feature entry #{} is not an object.", i);
          return false;
        }

      std::string feature_name;

      if (!read_value(feature_name, entry, "id"))
        return false;

      const std::optional<bim::game::feature_flags> flag =
          bim::game::from_simple_string(feature_name);

      if (!flag)
        {
          ic_log(iscool::log::nature::warning(), "config",
                 "Game feature entry #{} references unknown flag {}.", i,
                 feature_name);
          continue;
        }

      if (!read_value(result.game_feature_price[*flag], entry, "coins"))
        return false;
    }

  return true;
}

static bool parse_game_feature_slots(bim::app::config& result,
                                     const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    {
      ic_log(iscool::log::nature::error(), "config",
             "Missing game feature slots in config.");
      return false;
    }

  const Json::Value& entries = json[n];

  if (!entries.isArray())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Game feature slots is not an array.");
      return false;
    }

  for (Json::Value::ArrayIndex i = 0, count = entries.size(); i != count; ++i)
    {
      const Json::Value& entry = entries[i];

      if (!entry.isObject())
        {
          ic_log(iscool::log::nature::error(), "config",
                 "Game feature entry #{} is not an object.", i);
          return false;
        }

      std::size_t index;

      if (!read_value(index, entry, "index"))
        return false;

      if (index >= std::size(result.game_feature_slot_price))
        {
          ic_log(iscool::log::nature::warning(), "config",
                 "Game feature slot #{} references unknown slot #{}.", i,
                 index);
          continue;
        }

      if (!read_value(result.game_feature_slot_price[index], entry, "coins"))
        return false;
    }

  return true;
}

static bool parse_shop_products(bim::app::config& result,
                                const Json::Value& json, const char* n)
{
  if (!iscool::json::is_member(n, json))
    {
      ic_log(iscool::log::nature::error(), "config",
             "Missing shop in config.");
      return false;
    }

  const Json::Value& entries = json[n];

  if (!entries.isArray())
    {
      ic_log(iscool::log::nature::warning(), "config",
             "Shop products is not an array.");
      return false;
    }

  for (Json::Value::ArrayIndex i = 0, count = entries.size(); i != count; ++i)
    {
      const Json::Value& entry = entries[i];

      if (!entry.isObject())
        {
          ic_log(iscool::log::nature::error(), "config",
                 "Shop entry #{} is not an object.", i);
          return false;
        }

      std::string id;

      if (!read_value(id, entry, "id"))
        return false;

      int coins;

      if (!read_value(coins, entry, "coins"))
        return false;

      result.shop_products.emplace_back(std::move(id));
      result.shop_product_coins.emplace_back(coins);
    }

  return true;
}

bim::app::config::config()
  : most_recent_version(bim::version_major)
  , game_server("bim.jorge.st:"
                + std::to_string(20000 + bim::version_major * 100
                                 + bim::net::protocol_version))
  , version_update_interval(std::chrono::days(1))
{
  game_feature_price[bim::game::feature_flags::falling_blocks] = 50;
  game_feature_price[bim::game::feature_flags::shield] = 250;
  game_feature_price[bim::game::feature_flags::invisibility] = 250;
  game_feature_price[bim::game::feature_flags::fences] = 500;
  game_feature_price[bim::game::feature_flags::fog_of_war] = 1000;

  for (int i = 0; i != g_game_feature_slot_count; ++i)
    game_feature_slot_price[i] = i * 250;
}

std::optional<bim::app::config> bim::app::load_config(const Json::Value& json)
{
  if (!json.isObject())
    return std::nullopt;

  config result;

  if (!parse_misc(result, json, "misc"))
    return std::nullopt;

  if (!parse_server_list(result, json, "game_servers"))
    return std::nullopt;

  if (!parse_game_features(result, json, "game_features"))
    return std::nullopt;

  if (!parse_game_feature_slots(result, json, "game_feature_slots"))
    return std::nullopt;

  if (!parse_shop_products(result, json, "shop"))
    return std::nullopt;

  return result;
}
