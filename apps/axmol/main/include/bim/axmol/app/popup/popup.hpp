// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/named_node_group.hpp>

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/input/observer/key_sink_handle.hpp>
#include <bim/axmol/input/observer/touch_sink_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::axmol::app
{
  class main_scene;

  class popup
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((main_scene*)(main_scene))),
        ic_context_no_properties);

  public:
    popup(const context& context, const iscool::style::declaration& style);
    ~popup();

    void show(const bim::axmol::widget::named_node_group& nodes,
              const iscool::style::declaration& bounds,
              const bim::axmol::input::node_reference& inputs);
    void hide();

  private:
    bim_declare_controls_struct(controls, m_controls, 2);
    const iscool::style::declaration& m_style_bounds;
    const iscool::style::declaration& m_style_display_show;
    const iscool::style::declaration& m_style_action_show;

    bim::axmol::input::touch_sink_handle m_touch_sink;
    bim::axmol::input::key_sink_handle m_key_sink;
    bim::axmol::input::tree m_inputs;

    std::vector<ax::Node*> m_client_nodes;

    bim::axmol::action::runner m_action_runner;

    bool m_shown;
  };
}
