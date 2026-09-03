// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/net/message/game_id.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim
{
  namespace axmol::widget
  {
    class context;
  }

  namespace app
  {
    class analytics_service;
    class consume_game_reward_job;
    class player_profile;
  }

  namespace business
  {
    class request_headers;
  }

  namespace net
  {
    struct game_launch_event;
  }

  namespace game
  {
    class contest_result;
  }
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
  class wallet;

  class end_game
  {
    DECLARE_VOID_SIGNAL(quit, m_quit)
    DECLARE_VOID_SIGNAL(revenge, m_revenge)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::business::request_headers*)(request_headers))     //
            ((const bim::axmol::widget::context*)(widget_context))         //
            ((bim::app::analytics_service*)(analytics))                    //
            ((bim::app::player_profile*)(player_profile))                  //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ),
        ic_context_no_properties);

  public:
    end_game(const context& context, const iscool::style::declaration& style);
    ~end_game();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void game_started(const bim::net::game_launch_event& event);
    void displaying(const bim::game::contest_result& result);
    void displayed();
    void closing();

  private:
    enum class display_action : std::uint8_t;

  private:
    void reward_received(std::int64_t coins);

    void animate_wallet() const;

    void dispatch_revenge();
    void dispatch_quit();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 4);

    const std::unique_ptr<wallet> m_wallet;

    const iscool::style::declaration& m_action_draw;
    const iscool::style::declaration& m_action_win;
    const iscool::style::declaration& m_action_lose;

    const std::unique_ptr<bim::app::consume_game_reward_job>
        m_consume_reward_job;

    iscool::signals::connection m_reward_success_connection;

    bim::axmol::action::runner m_main_actions;

    bim::net::game_id m_game_id;
    std::uint8_t m_player_index;
    display_action m_display_action;
    bool m_displayed;

    bim::axmol::widget::named_node_group m_all_nodes;
  };
}
