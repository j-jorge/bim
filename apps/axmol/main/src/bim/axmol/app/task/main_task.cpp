// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/loading_screen.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/net/message/authentication_error_code.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/analytics_service.hpp>
#include <bim/app/preference/date_of_next_config_update.hpp>
#include <bim/app/preference/date_of_next_version_update_message.hpp>
#include <bim/app/preference/update_preferences.hpp>

#include <bim/version.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/files/full_path_exists.hpp>
#include <iscool/files/get_writable_path.hpp>
#include <iscool/files/rename_file.hpp>
#include <iscool/http/send.hpp>
#include <iscool/i18n/gettext.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/json/parse_string.hpp>
#include <iscool/json/write_to_stream.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/time/now.hpp>

#include <fmt/format.h>

#include <cstdlib>
#include <fstream>

static std::string cached_remote_config_file()
{
  return iscool::files::get_writable_path() + "/remote-config.json";
}

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, reset, m_reset);

bim::axmol::app::main_task::main_task(context context,
                                      const iscool::style::declaration& style)
  : m_context(context)
  , m_loading_screen(new loading_screen(
        m_context, *style.get_declaration("loading-screen")))
{
  m_context.set_session_handler(&m_session_handler);
  m_context.set_config(&m_config);
}

bim::axmol::app::main_task::~main_task() = default;

void bim::axmol::app::main_task::start()
{
  m_loader_connection = m_loading_screen->connect_to_done(
      [this]()
      {
        resources_loaded();
      });
  m_loading_screen->start();
}

void bim::axmol::app::main_task::resources_loaded()
{
  m_context.get_audio()->play_music("menu", iscool::audio::loop_mode::forever);

  if (load_config())
    start_optimistic();
  else
    start_fresh();
}

/// Set up the game by considering that the configuration is in a good state.
void bim::axmol::app::main_task::start_optimistic()
{
  m_is_forcing_config_update = false;
  bim::app::update_preferences(*m_context.get_local_preferences(), m_config);

  if (iscool::time::now<std::chrono::hours>()
      >= bim::app::date_of_next_config_update(
          *m_context.get_local_preferences()))
    fetch_remote_config();

  create_ui();

  if (!display_version_update_message())
    connect_to_game_server();
}

/**
 * Fetch the remote config and wait for it before going on with the
 * initialization.
 */
void bim::axmol::app::main_task::start_fresh()
{
  m_is_forcing_config_update = true;
  fetch_remote_config();
}

void bim::axmol::app::main_task::create_ui()
{
  m_style = iscool::style::loader::load("application");

  m_message_popup.reset(
      new message_popup(m_context, *m_style.get_declaration("message-popup")));

  m_screen_wheel.reset(
      new screen_wheel(m_context, *m_style.get_declaration("screen-wheel")));
  m_screen_wheel->connect_to_reset(
      [this]() -> void
      {
        m_reset();
      });

  m_loading_screen->stop();
}

bool bim::axmol::app::main_task::load_config()
{
  const std::string remote_config_file = cached_remote_config_file();

  if (iscool::files::full_path_exists(remote_config_file))
    {
      std::optional<bim::app::config> config =
          bim::app::load_config(iscool::json::from_file(remote_config_file));

      if (config)
        {
          m_config = std::move(*config);
          return true;
        }
    }

  return false;
}

void bim::axmol::app::main_task::config_ready()
{
  assert(m_is_forcing_config_update);
  update_preferences(*m_context.get_local_preferences(), m_config);

  create_ui();

  if (!display_version_update_message())
    connect_to_game_server();
}

void bim::axmol::app::main_task::fetch_remote_config()
{
  ic_log(iscool::log::nature::info(), "main_task", "Updating config.");

  auto on_result = [this](const std::vector<char>& response) -> void
  {
    validate_remote_config(std::string_view(response.begin(), response.end()));

    if (m_is_forcing_config_update)
      config_ready();
  };

  auto on_error = [this](const std::vector<char>& response) -> void
  {
    ic_log(iscool::log::nature::warning(), "main_task",
           "Failed to fetch remote config {}.",
           std::string(response.begin(), response.end()));

    if (m_is_forcing_config_update)
      config_ready();
  };

  m_config_request_connections = iscool::http::get(
      "https://bim.jorge.st/client-config.json", on_result, on_error);
}

void bim::axmol::app::main_task::validate_remote_config(
    const std::string_view& str)
{
  const Json::Value json_config = iscool::json::parse_string(std::string(str));

  if (!json_config)
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to parse remote config {}.", str);

      bim::app::error(*m_context.get_analytics(), "config-parse-error");
      return;
    }

  const std::optional<bim::app::config> config =
      bim::app::load_config(json_config);

  if (!config)
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to load remote config from Json {}.", str);
      bim::app::error(*m_context.get_analytics(), "config-load-error");
      return;
    }

  if (m_is_forcing_config_update)
    m_config = *config;

  const std::string tmp_path =
      iscool::files::get_writable_path() + "/remote-config.json.tmp";
  std::ofstream f(tmp_path);

  if (!iscool::json::write_to_stream(f, json_config))
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to save remote config {}.", str);
      return;
    }

  if (!iscool::files::rename_file(tmp_path, cached_remote_config_file()))
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to move remote config file to its final location.");
      return;
    }

  ic_log(iscool::log::nature::info(), "main_task", "Config updated.");

  bim::app::date_of_next_config_update(
      *m_context.get_local_preferences(),
      iscool::time::now<std::chrono::hours>()
          + config->remote_config_update_interval);
}

bool bim::axmol::app::main_task::display_version_update_message()
{
  if (m_config.most_recent_version <= bim::version_major)
    return false;

  const std::chrono::hours now = iscool::time::now<std::chrono::hours>();
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  if (now < bim::app::date_of_next_version_update_message(preferences))
    return false;

  bim::app::date_of_next_version_update_message(
      preferences, now + m_config.version_update_interval);

  m_message_connection = m_message_popup->connect_to_ok(
      [this]() -> void
      {
        m_message_connection.disconnect();
        connect_to_game_server();
      });

  m_message_popup->show(ic_gettext("A new version of Bim! is available! "
                                   "Please update as soon as possible."));

  return true;
}

void bim::axmol::app::main_task::connect_to_game_server()
{
  m_session_authentication_error_connection =
      m_session_handler.connect_to_authentication_error(
          [this](bim::net::authentication_error_code error_code)
          {
            const std::string error = std::to_string(
                std::underlying_type_t<bim::net::authentication_error_code>(
                    error_code));
            m_context.get_analytics()->event(
                "error", { { "cause", "authentication-error" },
                           { "error-code", error } });

            game_server_connection_error(error_code);
          });

  const char* const env_server = std::getenv("BIM_GAME_SERVER_HOST");
  std::string_view server_host;

  if (env_server)
    server_host = env_server;
  else
    server_host = m_config.game_server;

  m_session_handler.connect(server_host);
}

void bim::axmol::app::main_task::game_server_connection_error(
    bim::net::authentication_error_code error_code)
{
  if ((error_code == bim::net::authentication_error_code::bad_protocol)
      && !m_is_forcing_config_update)
    {
      ic_log(iscool::log::nature::info(), "main_task",
             "Bad protocol. Forcing a config update.");

      start_fresh();
    }
  else
    {
      m_message_popup->show(fmt::format(
          fmt::runtime(ic_gettext("Failed to authenticate with the "
                                  "game server. Error code {}.")),
          std::underlying_type_t<bim::net::authentication_error_code>(
              error_code)));
    }
}
