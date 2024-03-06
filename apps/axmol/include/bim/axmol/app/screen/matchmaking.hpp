#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/net/message/channel_id.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::net
{
  class new_game_exchange;
  class session_handler;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class matchmaking
  {
    DECLARE_VOID_SIGNAL(start_game, m_start_game)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((bim::net::session_handler*)(session_handler))),
        ic_context_no_properties);

  public:
    matchmaking(const context& context,
                const iscool::style::declaration& style);
    ~matchmaking();

    bim::axmol::input::node_reference input_node() const;
    // TODO: rename display_nodes
    const bim::axmol::widget::named_node_group& nodes() const;

    void displayed();
    void closing();

  private:
    void update_display_with_game_proposal(unsigned player_count);
    void update_display_waiting();

    void accept_game();
    void launch_game(iscool::net::channel_id channel, unsigned player_count,
                     unsigned player_index);

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 2);

    std::unique_ptr<bim::net::new_game_exchange> m_new_game;

    iscool::signals::scoped_connection m_game_proposal_connection;
    iscool::signals::scoped_connection m_launch_connection;

    const iscool::style::declaration& m_style_displaying;
    const iscool::style::declaration& m_style_new_game;
    const iscool::style::declaration& m_style_wait;
  };
}
