// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/config.hpp>

#include <bim/net/session_handler.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>
#include <iscool/signals/shared_connection_set.hpp>
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
        ic_context_declare_properties(                      //
            ((bim::net::session_handler*)(session_handler)) //
            ((bim::app::config*)(config))));

  public:
    main_task(context context, const iscool::style::declaration& style);
    ~main_task();

    void start();

  private:
    struct steps;

  private:
    void resources_loaded();

    void try_create_ui();
    void create_ui();

    void fetch_remote_config();
    void validate_remote_config(const std::string_view& str);
    void load_local_config();
    void config_ready();

    bool display_version_update_message();
    void connect_to_game_server();

    void game_server_connection_error(
        bim::net::authentication_error_code error_code);

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

    iscool::signals::shared_connection_set m_config_request_connections;

    bim::app::config m_config;

    std::uint8_t m_done_steps;
  };
}
