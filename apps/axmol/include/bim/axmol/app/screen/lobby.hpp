#pragma once

#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>

namespace bim::axmol::widget
{
  class context;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class lobby
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties( //
            ((const bim::axmol::widget::context&)(widget_context))),
        ic_context_no_properties);

  public:
    lobby(const context& context, const iscool::style::declaration& style);
    ~lobby();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& nodes() const;

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 1);
  };
}
