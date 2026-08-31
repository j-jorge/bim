// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/fetch_config_job.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/business_url.hpp>
#include <bim/app/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/version.hpp>

#include <iscool/files/full_path_exists.hpp>
#include <iscool/files/get_writable_path.hpp>
#include <iscool/files/rename_file.hpp>
#include <iscool/http/json/send.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/json/write_to_stream.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

#include <fstream>
#include <sstream>

static std::string cached_remote_config_file()
{
  return iscool::files::get_writable_path() + "/remote-config.json";
}

IMPLEMENT_SIGNAL(bim::app::fetch_config_job, done, m_done)

bim::app::fetch_config_job::fetch_config_job(analytics_service& analytics)
  : m_analytics(analytics)
  , m_config(nullptr)
{}

void bim::app::fetch_config_job::start(config& cfg)
{
  ic_log(iscool::log::nature::info(), "fetch_config_job", "Start.");

  m_config = &cfg;

  auto on_result = [this](const Json::Value& response) -> void
    {
      validate_remote_config(response);
      config_ready();
    };

  auto on_error = [this](int status, std::span<const char> response) -> void
    {
      ic_log(iscool::log::nature::warning(), "fetch_config_job",
             "Failed to fetch remote config ({}) {}.", status, response);

      load_local_config();
      config_ready();
    };

  Json::Value body;
  body["client_version_major"] = bim::version_major;
  body["game_server_protocol_version"] = bim::net::protocol_version;

  m_connection = iscool::http::json::post(
      BIM_BUSINESS_SERVER_URL "/client/config", body, on_result, on_error);
}

void bim::app::fetch_config_job::validate_remote_config(
    const Json::Value& json_config)
{
  if (!json_config)
    {
      ic_log(iscool::log::nature::warning(), "fetch_config_job",
             "Remote config is null.");

      error(m_analytics, "config-parse-error");
      return;
    }

  const std::optional<config> config = load_config(json_config);

  if (!config)
    {
      error(m_analytics, "config-load-error");

      std::ostringstream oss;
      iscool::json::write_to_stream(oss, json_config);

      ic_log(iscool::log::nature::warning(), "fetch_config_job",
             "Failed to load remote config from Json {}.", oss.str());

      return;
    }

  *m_config = *config;

  const std::string tmp_path =
      iscool::files::get_writable_path() + "/remote-config.json.tmp";
  std::ofstream f(tmp_path);

  if (!iscool::json::write_to_stream(f, json_config))
    {
      ic_log(iscool::log::nature::warning(), "fetch_config_job",
             "Failed to save remote config.");
      return;
    }

  if (!iscool::files::rename_file(tmp_path, cached_remote_config_file()))
    {
      ic_log(iscool::log::nature::warning(), "fetch_config_job",
             "Failed to move remote config file to its final location.");
      return;
    }

  ic_log(iscool::log::nature::info(), "fetch_config_job", "Config updated.");
}

void bim::app::fetch_config_job::load_local_config()
{
  const std::string remote_config_file = cached_remote_config_file();

  if (!iscool::files::full_path_exists(remote_config_file))
    return;

  std::optional<config> config =
      load_config(iscool::json::from_file(remote_config_file));

  if (!config)
    return;

  *m_config = std::move(*config);
}

void bim::app::fetch_config_job::config_ready()
{
  m_config = nullptr;
  m_done();
}
