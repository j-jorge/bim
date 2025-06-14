// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <memory>
#include <string>

namespace bim
{
  namespace axmol::widget
  {
    class context;
  }

  namespace net
  {
    class keep_alive_exchange;
    class session_handler;
    struct game_launch_event;
  }

  namespace game
  {
    class contest_result;
  }
}

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

  namespace style
  {
    class declaration;
  }

  namespace system
  {
    class haptic_feedback;
  }
}

namespace bim::axmol::app
{
  class end_game;
  class lobby;
  class main_scene;
  class matchmaking;
  class message_popup;
  class online_game;
  class player_progress_tracker;
  class scene_lock;
  struct config;

  class screen_wheel
  {
    DECLARE_VOID_SIGNAL(reset, m_reset)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((const config*)(config))                                      //
            ((bim::net::session_handler*)(session_handler))                //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::social::service*)(social))                           //
            ((iscool::system::haptic_feedback*)(haptic_feedback))          //
            ((bool)(enable_debug))),
        ic_context_declare_properties(
            ((player_progress_tracker*)(player_progress_tracker))));

  public:
    screen_wheel(const context& context,
                 const iscool::style::declaration& style);
    ~screen_wheel();

  private:
    void map_nodes(ax::Node& container,
                   const bim::axmol::widget::named_node_group& nodes,
                   const iscool::style::declaration& style,
                   const std::string& bounds_style_name) const;

    void wire_permanent_connections();
    void connect_keep_alive();

    void switch_view(ax::Node& new_view);

    void animate_lobby_to_matchmaking();
    void animate_matchmaking_to_game(const bim::net::game_launch_event& event);
    void animate_matchmaking_to_lobby();
    void animate_game_to_end_game(const bim::game::contest_result& result);
    void animate_end_game_to_lobby();
    void animate_end_game_to_matchmaking();

    void lobby_displayed();
    void matchmaking_displayed();
    void online_game_displayed();
    void end_game_displayed();

    void disconnected();

  private:
    bim::axmol::ref_ptr<ax::Node> m_main_container;
    bim::axmol::input::tree m_inputs;
    ax::Node* m_active_view;

    bim_declare_controls_struct(controls, m_controls, 4);

    std::unique_ptr<player_progress_tracker> m_player_progress_tracker;
    std::unique_ptr<lobby> m_lobby;
    std::unique_ptr<matchmaking> m_matchmaking;
    std::unique_ptr<online_game> m_online_game;
    std::unique_ptr<end_game> m_end_game;

    std::unique_ptr<message_popup> m_message_popup;

    std::unique_ptr<bim::net::keep_alive_exchange> m_keep_alive;
    iscool::signals::scoped_connection m_keep_alive_connection;

    bool m_silently_reconnect;
  };
}
