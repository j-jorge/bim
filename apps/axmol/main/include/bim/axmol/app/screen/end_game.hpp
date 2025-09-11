// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim
{
  namespace axmol::widget
  {
    class context;
  }

  namespace game
  {
    class contest_result;
  }

  namespace net
  {
    struct game_launch_event;
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
  class analytics_service;
  class wallet;
  struct config;

  class end_game
  {
    DECLARE_VOID_SIGNAL(quit, m_quit)
    DECLARE_VOID_SIGNAL(revenge, m_revenge)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((analytics_service*)(analytics))                              //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((const config*)(config))                                      //
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
    void dispatch_revenge();
    void dispatch_quit();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 4);

    const std::unique_ptr<wallet> m_wallet;

    const iscool::style::declaration& m_action_draw;
    const iscool::style::declaration& m_action_win;
    const iscool::style::declaration& m_action_lose;

    bim::axmol::action::runner m_main_actions;

    std::uint8_t m_player_index;

    bim::axmol::widget::named_node_group m_all_nodes;
  };
}
