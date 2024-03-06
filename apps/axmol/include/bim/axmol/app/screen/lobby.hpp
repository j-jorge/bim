#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::net
{
  class session_handler;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class lobby
  {
    DECLARE_VOID_SIGNAL(play, m_play)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((bim::net::session_handler*)(session_handler))),
        ic_context_no_properties);

  public:
    lobby(const context& context, const iscool::style::declaration& style);
    ~lobby();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& nodes() const;

    void displayed();
    void closing();

  private:
    void apply_connected_state();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 1);

    iscool::signals::scoped_connection m_session_connection;
  };
}
