// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/loading_screen.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/net/message/authentication_error_code.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/analytics_service.hpp>
#include <bim/app/business/legacy_state.hpp>
#include <bim/app/business/player_profile_json.hpp>
#include <bim/app/business_url.hpp>
#include <bim/app/preference/date_of_next_version_update_message.hpp>
#include <bim/app/preference/device_id.hpp>
#include <bim/app/preference/legacy.hpp>
#include <bim/app/preference/update_preferences.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <bim/business/post.hpp>

#include <bim/game/feature_flags_string.hpp>

#include <bim/dev.hpp>
#include <bim/tracy.hpp>
#include <bim/version.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/files/full_path_exists.hpp>
#include <iscool/files/get_writable_path.hpp>
#include <iscool/files/rename_file.hpp>
#include <iscool/http/json/send.hpp>
#include <iscool/i18n/gettext.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/json/write_to_stream.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/time/now.hpp>

#include <json/value.h>

#include <fmt/format.h>

#include <cstdlib>
#include <fstream>

/*
  The opening process looks like this:

 load_resources       fetch_remote_config        connect_to_business_server
create_minimal_ui   display_version_update                   |
        |                       |                            |
        |                       +--->+<----------------------+
        |                            |                       |
        |                 connect_to_game_server             |
        |                                                    |
        |                                            push_legacy_state
        |                                          fetch_player_profile
        |                                                    |
        +-------------------------+--------------------------+
                                  |
                            create_main_ui
                                 open
*/
struct bim::axmol::app::main_task::steps
{
  static constexpr std::uint8_t resources = 1 << 0;
  static constexpr std::uint8_t config = 1 << 1;
  static constexpr std::uint8_t business = 1 << 2;
  static constexpr std::uint8_t player_profile = 1 << 3;
};

static std::string cached_remote_config_file()
{
  return iscool::files::get_writable_path() + "/remote-config.json";
}

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, reset, m_reset);

bim::axmol::app::main_task::main_task(context context,
                                      const iscool::style::declaration& style)
  : m_context(std::move(context))
  , m_loading_screen(new loading_screen(
        m_context, *style.get_declaration("loading-screen")))
  , m_player_profile_url(bim::app::business_url + "/client/me")
  , m_legacy_state_url(bim::app::business_url
                       + "/client/transfer-legacy-inventory")
{}

bim::axmol::app::main_task::~main_task() = default;

void bim::axmol::app::main_task::start()
{
  m_done_steps = 0;

  m_loader_connection = m_loading_screen->connect_to_done(
      [this]()
        {
          resources_loaded();
        });
  m_loading_screen->start();

  fetch_remote_config();
  connect_to_business_server();
}

bool bim::axmol::app::main_task::check_done_steps(std::uint8_t steps) const
{
  return (m_done_steps && steps) == steps;
}

void bim::axmol::app::main_task::resources_loaded()
{
  m_done_steps |= steps::resources;
  m_context.get_audio()->play_music("menu", iscool::audio::loop_mode::forever);

  create_minimal_ui();

  try_open();
}

void bim::axmol::app::main_task::create_minimal_ui()
{
  ZoneScoped;

  m_style = iscool::style::loader::load("application");

  m_message_popup.reset(
      new message_popup(m_context, *m_style.get_declaration("message-popup")));
  m_message_connection = m_message_popup->connect_to_ok(
      [this]() -> void
        {
          try_show_message();
        });

  try_show_message();
}

void bim::axmol::app::main_task::create_main_ui()
{
  ZoneScoped;

  m_context.set_session_handler(&m_session_handler);
  m_context.set_config(&m_config);
  m_context.set_request_headers(&m_request_headers);
  m_context.set_player_profile(&m_player_profile);

  m_screen_wheel.reset(
      new screen_wheel(m_context, *m_style.get_declaration("screen-wheel")));
  m_screen_wheel->connect_to_reset(
      [this]() -> void
        {
          m_reset();
        });

  m_loading_screen->stop();
}

void bim::axmol::app::main_task::fetch_remote_config()
{
  ic_log(iscool::log::nature::info(), "main_task", "Updating config.");

  auto on_result = [this](const Json::Value& response) -> void
    {
      validate_remote_config(response);
      config_ready();
    };

  auto on_error = [this](int status, std::span<const char> response) -> void
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to fetch remote config ({}) {}.", status, response);

      load_local_config();
      config_ready();
    };

  Json::Value body;
  body["client_version_major"] = bim::version_major;
  body["game_server_protocol_version"] = bim::net::protocol_version;

  m_config_connections = iscool::http::json::post(
      bim::app::business_url + "/client/config", body, on_result, on_error);
}

void bim::axmol::app::main_task::validate_remote_config(
    const Json::Value& json_config)
{
  if (!json_config)
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Remote config is null.");

      bim::app::error(*m_context.get_analytics(), "config-parse-error");
      return;
    }

  const std::optional<bim::app::config> config =
      bim::app::load_config(json_config);

  if (!config)
    {
      bim::app::error(*m_context.get_analytics(), "config-load-error");

      std::ostringstream oss;
      iscool::json::write_to_stream(oss, json_config);

      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to load remote config from Json {}.", oss.str());

      return;
    }

  m_config = *config;

  const std::string tmp_path =
      iscool::files::get_writable_path() + "/remote-config.json.tmp";
  std::ofstream f(tmp_path);

  if (!iscool::json::write_to_stream(f, json_config))
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to save remote config.");
      return;
    }

  if (!iscool::files::rename_file(tmp_path, cached_remote_config_file()))
    {
      ic_log(iscool::log::nature::warning(), "main_task",
             "Failed to move remote config file to its final location.");
      return;
    }

  ic_log(iscool::log::nature::info(), "main_task", "Config updated.");
}

void bim::axmol::app::main_task::load_local_config()
{
  const std::string remote_config_file = cached_remote_config_file();

  if (!iscool::files::full_path_exists(remote_config_file))
    return;

  std::optional<bim::app::config> config =
      bim::app::load_config(iscool::json::from_file(remote_config_file));

  if (!config)
    return;

  m_config = std::move(*config);
}

void bim::axmol::app::main_task::config_ready()
{
  bim::app::update_preferences(*m_context.get_local_preferences());

  m_done_steps |= steps::config;

  try_connect_to_game_server();
  display_version_update_message();
}

void bim::axmol::app::main_task::display_version_update_message()
{
  if (m_config.most_recent_version <= bim::version_major)
    return;

  const std::chrono::hours now = iscool::time::now<std::chrono::hours>();
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  if (now < bim::app::date_of_next_version_update_message(preferences))
    return;

  bim::app::date_of_next_version_update_message(
      preferences, now + m_config.version_update_interval);

  queue_message(ic_gettext("A new version of Bim! is available! "
                           "Please update as soon as possible."));
}

void bim::axmol::app::main_task::connect_to_business_server()
{
  ic_log(iscool::log::nature::info(), "main_task",
         "Connecting to business server at '{}'.", bim::app::business_url);

  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  bim::app::ensure_device_id_exists(preferences);

  Json::Value body;
  body["device_id"] = bim::app::device_id(preferences);

  const auto on_result = [this](const Json::Value& r)
    {
      business_server_connection_success(r);
    };
  const auto on_error = [this](int status, std::span<const char> body)
    {
      business_server_connection_error(status, body);
    };

  m_connections =
      iscool::http::json::post(bim::app::business_url + "/client/authenticate",
                               body, on_result, on_error);
}

void bim::axmol::app::main_task::business_server_connection_success(
    const Json::Value& r)
{
  m_session_token = iscool::json::cast<std::string>(r["session_token"]);

  ic_log(iscool::log::nature::info(), "main_task",
         "Connected to business server. Session token is '{}'.",
         m_session_token);

  m_done_steps |= steps::business;

  try_connect_to_game_server();
  m_request_headers.push_authorization(m_session_token);

  if (bim::app::legacy_inventory_pushed(*m_context.get_local_preferences()))
    fetch_player_profile();
  else
    push_legacy_state();
}

void bim::axmol::app::main_task::business_server_connection_error(
    int status, std::span<const char> body)
{
  ic_log(iscool::log::nature::error(), "main_task",
         "Failed to connect to the business server ({}): {}.", status,
         std::string_view(body.begin(), body.end()).substr(0, 1024));

  bim::app::error(*m_context.get_analytics(), "business-authentication");

  queue_message(
      fmt::format(fmt::runtime(ic_gettext("Failed to authenticate with the "
                                          "business server. Status {}.")),
                  status));

  // It won't work but we'll still continue toward opening the main screen.
  fetch_player_profile();
}

void bim::axmol::app::main_task::try_connect_to_game_server()
{
  const std::uint8_t needed_steps = steps::config | steps::business;

  if ((m_done_steps & needed_steps) == needed_steps)
    connect_to_game_server();
}

void bim::axmol::app::main_task::connect_to_game_server()
{
  m_session_authentication_error_connection =
      m_session_handler.connect_to_authentication_error(
          [this](bim::net::authentication_error_code error_code)
            {
              game_server_connection_error(error_code);
            });

  ic_log(iscool::log::nature::info(), "main_task",
         "Connecting to game server at '{}'.", m_config.game_server);

  m_session_handler.connect(m_config.game_server, m_session_token);
}

void bim::axmol::app::main_task::game_server_connection_error(
    bim::net::authentication_error_code error_code)
{
  ic_log(
      iscool::log::nature::error(), "main_task",
      "Failed to authenticate with the game server. Error code {}.",
      std::underlying_type_t<bim::net::authentication_error_code>(error_code));

  const std::string error = std::to_string(
      std::underlying_type_t<bim::net::authentication_error_code>(error_code));
  m_context.get_analytics()->event(
      "error",
      { { "cause", "authentication-error" }, { "error-code", error } });

  // Careful here, this is called even if we have already opened the main
  // scene.
  queue_message(
      fmt::format(fmt::runtime(ic_gettext("Failed to authenticate with the "
                                          "game server. Error code {}.")),
                  std::underlying_type_t<bim::net::authentication_error_code>(
                      error_code)));
}

void bim::axmol::app::main_task::push_legacy_state()
{
  ic_log(iscool::log::nature::info(), "main_task",
         "Sending the legacy state to the backend.");

  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  Json::Value body;
  body["coins"] = preferences.get_value("coins", (std::int64_t)0);

  {
    Json::Value& slots = body["slots"];

    if (preferences.get_value("feature_flags.slot_0.available", false))
      slots.append(0);

    if (preferences.get_value("feature_flags.slot_1.available", false))
      slots.append(1);
  }

  {
    Json::Value& game_features = body["game_features"];
    const bim::game::feature_flags owned_features =
        (bim::game::feature_flags)preferences.get_value(
            "feature_flags.available",
            std::int64_t(bim::game::feature_flags{}));

    for (const bim::game::feature_flags f :
         bim::game::g_all_game_feature_flags)
      if (!!(owned_features & f))
        game_features.append(std::string(bim::game::to_simple_string(f)));
  }

  {
    Json::Value& game_feature_selection = body["game_feature_selection"];

    {
      const bim::game::feature_flags f =
          (bim::game::feature_flags)preferences.get_value(
              "feature_flags.slot_0",
              std::int64_t(bim::game::feature_flags{}));
      Json::Value& selection =
          game_feature_selection[game_feature_selection.size()];

      selection["slot_index"] = 0;
      selection["feature"] = std::string(bim::game::to_simple_string(f));
    }

    {
      const bim::game::feature_flags f =
          (bim::game::feature_flags)preferences.get_value(
              "feature_flags.slot_1",
              std::int64_t(bim::game::feature_flags{}));
      Json::Value& selection =
          game_feature_selection[game_feature_selection.size()];

      selection["slot_index"] = 1;
      selection["feature"] = std::string(bim::game::to_simple_string(f));
    }
  }

  {
    Json::Value& arena_stats = body["arena_stats"];

    arena_stats["game_count"] =
        preferences.get_value("arena_stats.game_count", (std::int64_t)0);
    arena_stats["victory_count"] =
        preferences.get_value("arena_stats.victory_count", (std::int64_t)0);
    arena_stats["defeat_count"] =
        preferences.get_value("arena_stats.defeat_count", (std::int64_t)0);
  }

  m_connections =
      bim::business::post<bim::app::legacy_state_transfer_response>(
          m_legacy_state_url, m_request_headers.headers, body,
          [this](const bim::app::legacy_state_transfer_response& response)
            {
              push_legacy_state_success(response);
            },
          [this]()
            {
              push_legacy_state_error();
            });
}

void bim::axmol::app::main_task::push_legacy_state_success(
    const bim::app::legacy_state_transfer_response& response)
{
  ic_log(iscool::log::nature::info(), "main_task", "Legacy state sent: {}.",
         (std::int64_t)response.transfer_state);

  if ((response.transfer_state == bim::app::transfer_result::done)
      || (response.transfer_state == bim::app::transfer_result::already_done))
    bim::app::legacy_inventory_pushed(*m_context.get_local_preferences(),
                                      true);

  fetch_player_profile();
}

void bim::axmol::app::main_task::push_legacy_state_error()
{
  ic_log(iscool::log::nature::error(), "main_task",
         "Failed to send the legacy state to the backend.");
  fetch_player_profile();
}

void bim::axmol::app::main_task::fetch_player_profile()
{
  ic_log(iscool::log::nature::info(), "main_task",
         "Fetching the player's profile.");

  m_connections = bim::business::post(
      m_player_profile, m_player_profile_url, m_request_headers.headers,
      [this]()
        {
          fetch_player_profile_success();
        },
      [this]()
        {
          fetch_player_profile_error();
        });
}

void bim::axmol::app::main_task::fetch_player_profile_success()
{
  ic_log(iscool::log::nature::info(), "main_task", "Player profile received.");

  m_done_steps |= steps::player_profile;
  try_open();
}

void bim::axmol::app::main_task::fetch_player_profile_error()
{
  ic_log(iscool::log::nature::error(), "main_task",
         "Failed to fetch the player's profile.");

  m_player_profile = {};
  m_done_steps |= steps::player_profile;
  try_open();
}

void bim::axmol::app::main_task::try_open()
{
  const std::uint8_t needed_steps = steps::player_profile | steps::resources;

  if ((m_done_steps & needed_steps) == needed_steps)
    create_main_ui();
}

void bim::axmol::app::main_task::queue_message(std::string message)
{
  m_messages.emplace_back(std::move(message));
  try_show_message();
}

void bim::axmol::app::main_task::try_show_message()
{
  if (!m_message_popup || m_messages.empty() || m_message_popup->is_shown())
    return;

  const std::string message = std::move(m_messages[0]);
  m_messages.erase(m_messages.begin());

  m_message_popup->show(message);
}
