#pragma once

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
    class game_launch_event;
  }
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class end_game
  {
    DECLARE_VOID_SIGNAL(quit, m_quit)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties( //
            ((const bim::axmol::widget::context&)(widget_context))),
        ic_context_no_properties);

  public:
    end_game(const context& context, const iscool::style::declaration& style);
    ~end_game();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& nodes() const;

    void game_started(const bim::net::game_launch_event& event);
    void displaying(const bim::game::contest_result& result);
    void displayed();
    void closing();

  private:
    void apply_connected_state();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 2);

    std::uint8_t m_player_index;
  };
}
