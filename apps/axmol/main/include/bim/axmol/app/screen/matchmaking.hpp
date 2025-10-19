// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/context.hpp>
#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/net/message/channel_id.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::app
{
  struct config;
  class matchmaking_wait_message;
}

namespace bim::net
{
  class new_game_exchange;
  class session_handler;
  struct game_launch_event;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class analytics_service;
  class wallet;

  class matchmaking
  {
    DECLARE_SIGNAL(void(const bim::net::game_launch_event&), start_game,
                   m_start_game)
    DECLARE_VOID_SIGNAL(back, m_back)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((analytics_service*)(analytics))                              //
            ((bim::net::session_handler*)(session_handler))                //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((const bim::app::config*)(config))                            //
            ),
        ic_context_no_properties);

  public:
    matchmaking(const context& context,
                const iscool::style::declaration& style);
    ~matchmaking();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void attached();
    void displaying();
    void displayed();
    void closing();

  private:
    void update_display_with_game_proposal(unsigned player_count);
    void run_actions(bim::axmol::action::runner& runner,
                     const iscool::style::declaration& style) const;

    void accept_game();
    void launch_game(const bim::net::game_launch_event& event);

    void open_discord() const;

    void dispatch_back() const;

  private:
    ic_declare_state_monitor(m_player_count_monitor);
    ic_declare_state_monitor(m_launch_monitor);

    bim::axmol::input::single_key_observer_handle m_escape;
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 4);

    const std::unique_ptr<wallet> m_wallet;

    std::unique_ptr<bim::net::new_game_exchange> m_new_game;

    iscool::signals::scoped_connection m_game_proposal_connection;
    iscool::signals::scoped_connection m_launch_connection;

    std::unique_ptr<bim::app::matchmaking_wait_message> m_wait_message;

    const iscool::style::declaration& m_style_displaying;
    const iscool::style::declaration& m_action_displaying;

    const iscool::style::declaration& m_action_wait;
    const iscool::style::declaration& m_action_2_players;
    const iscool::style::declaration& m_action_3_players;
    const iscool::style::declaration& m_action_4_players;

    bim::axmol::action::runner m_main_actions;
    bim::axmol::action::runner m_state_actions;

    bim::axmol::widget::named_node_group m_all_nodes;
  };
}
