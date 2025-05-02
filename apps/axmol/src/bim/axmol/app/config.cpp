// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/version.hpp>

#include <iscool/json/cast_int.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/cast_uint.hpp>
#include <iscool/json/is_member.hpp>
#include <iscool/json/is_of_type_string.hpp>
#include <iscool/json/is_of_type_uint.hpp>

#include <type_traits>

static bool parse_server_list(bim::axmol::app::config& result,
                              const Json::Value& servers)
{
  if (!servers.isArray())
    return false;

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
{}

std::optional<bim::axmol::app::config>
bim::axmol::app::load_config(const Json::Value& json)
{
  if (!json.isObject())
    return std::nullopt;

  config result;

  result.most_recent_version =
      iscool::json::cast<unsigned int>(json["mrv"], bim::version_major);

  const auto read_value = [&json]<typename T>(T& r, const char* n) -> bool
  {
    if (!iscool::json::is_member(n, json))
      return true;

    const Json::Value& v = json[n];

    if (!iscool::json::is_of_type<T>(v))
      return false;

    r = iscool::json::cast<T>(v);

    return true;
  };

  const auto read_hours = [&read_value](std::chrono::hours& r,
                                        const char* n) -> bool
  {
    std::uint64_t h = r.count();

    if (read_value(h, n))
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

  if (!read_value(result.coins_per_victory, "cpv"))
    return std::nullopt;

  if (!read_value(result.coins_per_defeat, "cpde"))
    return std::nullopt;

  if (!read_value(result.coins_per_draw, "cpdr"))
    return std::nullopt;

  return result;
}
