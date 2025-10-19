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

namespace bim::axmol::app
{
  class analytics_service;
  class main_scene;
  class message_popup;
  class scene_lock;
  class screen_wheel;

  class main_task
  {
    DECLARE_VOID_SIGNAL(end, m_end)
    DECLARE_VOID_SIGNAL(reset, m_reset)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((scene_lock*)(scene_lock))                                    //
            ((analytics_service*)(analytics))                              //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::social::service*)(social))                           //
            ((iscool::system::haptic_feedback*)(haptic_feedback))          //
            ((bool)(enable_debug))),
        ic_context_declare_properties(                      //
            ((bim::net::session_handler*)(session_handler)) //
            ((bim::app::config*)(config))));

  public:
    explicit main_task(context context);
    ~main_task();

    void start();

  private:
    void start_optimistic();
    void start_fresh();
    void create_ui();

    bool load_config();
    void config_ready();
    void fetch_remote_config();
    void validate_remote_config(const std::string_view& str);

    void read_translations();

    bool display_version_update_message();
    void connect_to_game_server();

    void game_server_connection_error(
        bim::net::authentication_error_code error_code);

  private:
    iscool::style::declaration m_style;
    bim::net::session_handler m_session_handler;
    std::unique_ptr<message_popup> m_message_popup;
    std::unique_ptr<screen_wheel> m_screen_wheel;

    iscool::signals::scoped_connection
        m_session_authentication_error_connection;
    iscool::signals::scoped_connection m_message_connection;

    iscool::signals::shared_connection_set m_config_request_connections;

    bim::app::config m_config;

    bool m_is_forcing_config_update;
  };
}
