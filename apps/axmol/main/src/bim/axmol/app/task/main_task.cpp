// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/loading_screen.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/net/message/authentication_error_code.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/analytics_service.hpp>
#include <bim/app/device_id.hpp>
#include <bim/app/job/fetch_player_profile_job.hpp>
#include <bim/app/job/push_legacy_state_job.hpp>
#include <bim/app/preference/date_of_next_version_update_message.hpp>
#include <bim/app/preference/update_preferences.hpp>

#include <bim/tracy.hpp>
#include <bim/version.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/i18n/gettext.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/time/now.hpp>

#include <fmt/format.h>

#include <cstdlib>

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

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, reset, m_reset);

bim::axmol::app::main_task::main_task(context context,
                                      const iscool::style::declaration& style)
  : m_context(std::move(context))
  , m_device_id(bim::app::device_id())
  , m_loading_screen(new loading_screen(
        m_context, *style.get_declaration("loading-screen")))
  , m_fetch_config(*m_context.get_analytics())
  , m_authenticate_with_business(*m_context.get_analytics())
{
  m_fetch_config.connect_to_done(
      [this]()
        {
          config_ready();
        });
  m_authenticate_with_business.connect_to_done(
      [this](std::string session_token)
        {
          business_server_connection_success(std::move(session_token));
        });
  m_authenticate_with_business.connect_to_error(
      [this](int status)
        {
          business_server_connection_error(status);
        });
}

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

  m_fetch_config.start(m_config);
  m_authenticate_with_business.start(m_device_id);
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

void bim::axmol::app::main_task::business_server_connection_success(
    std::string session_token)
{
  m_session_token = std::move(session_token);
  m_done_steps |= steps::business;

  try_connect_to_game_server();
  m_request_headers.push_authorization(m_session_token);

  start_authenticated_jobs();
}

void bim::axmol::app::main_task::business_server_connection_error(int status)
{
  queue_message(
      fmt::format(fmt::runtime(ic_gettext("Failed to authenticate with the "
                                          "business server. Status {}.")),
                  status));

  // It won't work but we'll still continue toward opening the main screen.
  start_authenticated_jobs();
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

void bim::axmol::app::main_task::start_authenticated_jobs()
{
  m_push_legacy_state.reset(new bim::app::push_legacy_state_job(
      *m_context.get_analytics(), m_request_headers,
      *m_context.get_local_preferences()));
  m_fetch_player_profile.reset(new bim::app::fetch_player_profile_job(
      *m_context.get_analytics(), m_request_headers));

  m_fetch_player_profile->connect_to_done(
      [this]()
        {
          m_done_steps |= steps::player_profile;
          try_open();
        });

  m_push_legacy_state->connect_to_done(
      [this]()
        {
          m_fetch_player_profile->start(m_player_profile);
        });

  m_push_legacy_state->start();
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
