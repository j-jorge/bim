// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/business/player_profile.hpp>
#include <bim/app/config.hpp>

#include <bim/business/request_headers.hpp>

#include <bim/net/session_handler.hpp>

#include <iscool/context.hpp>
#include <iscool/http/request_connection.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>
#include <iscool/style/declaration.hpp>

#include <cstdint>
#include <memory>

namespace iscool
{
  namespace audio
  {
    class mixer;
  }

  namespace preferences
  {
    class local_preferences;
  }

  namespace social
  {
    class service;
  }

  namespace system
  {
    class haptic_feedback;
  }
}

namespace bim::net
{
  enum class authentication_error_code : std::uint8_t;
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::app
{
  class analytics_service;
}

namespace Json
{
  class Value;
}

namespace bim::axmol::app
{
  class application_event_dispatcher;
  class loading_screen;
  class main_scene;
  class message_popup;
  class screen_wheel;

  class main_task
  {
    DECLARE_VOID_SIGNAL(reset, m_reset)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((bim::axmol::widget::context*)(widget_context))               //
            ((main_scene*)(main_scene))                                    //
            ((bim::app::analytics_service*)(analytics))                    //
            ((application_event_dispatcher*)(event_dispatcher))            //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::social::service*)(social))                           //
            ((iscool::system::haptic_feedback*)(haptic_feedback))          //
            ((bool)(enable_debug))),
        ic_context_declare_properties(                                 //
            ((bim::net::session_handler*)(session_handler))            //
            ((bim::app::config*)(config))                              //
            ((const bim::business::request_headers*)(request_headers)) //
            ((bim::app::player_profile*)(player_profile))              //
            ));

  public:
    main_task(context context, const iscool::style::declaration& style);
    ~main_task();

    void start();

  private:
    struct steps;

  private:
    bool check_done_steps(std::uint8_t steps) const;

    void resources_loaded();

    void try_create_minimal_ui();
    void create_minimal_ui();
    void create_main_ui();

    void fetch_remote_config();
    void validate_remote_config(const Json::Value& json_config);
    void load_local_config();
    void config_ready();

    void display_version_update_message();

    void connect_to_business_server();
    void business_server_connection_success(const Json::Value& response);
    void business_server_connection_error(int status,
                                          std::span<const char> body);

    void connect_to_game_server(const std::string& session_token);
    void game_server_connection_error(
        bim::net::authentication_error_code error_code);

    void fetch_player_profile();
    void fetch_player_profile_success();
    void fetch_player_profile_error();

    void try_open();

    void queue_message(std::string message);
    void try_show_message();

  private:
    iscool::style::declaration m_style;
    std::unique_ptr<loading_screen> m_loading_screen;

    bim::net::session_handler m_session_handler;
    std::unique_ptr<message_popup> m_message_popup;
    std::unique_ptr<screen_wheel> m_screen_wheel;

    iscool::signals::scoped_connection m_loader_connection;
    iscool::signals::scoped_connection
        m_session_authentication_error_connection;
    iscool::signals::scoped_connection m_message_connection;

    bim::business::request_headers m_request_headers;
    iscool::http::request_connection m_connections;

    bim::app::config m_config;
    bim::app::player_profile m_player_profile;

    std::uint8_t m_done_steps;

    /// Pending messages to be shown to the user.
    std::vector<std::string> m_messages;

    std::string m_player_profile_url;
  };
}
