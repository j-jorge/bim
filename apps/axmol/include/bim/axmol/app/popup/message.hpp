// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/observer/key_sink_handle.hpp>
#include <bim/axmol/input/observer/touch_sink_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>

#include <string_view>

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

  class message_popup
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((main_scene*)(main_scene))),
        ic_context_no_properties);

  public:
    message_popup(const context& context,
                  const iscool::style::declaration& style);
    ~message_popup();

    void show(std::string_view message);

  private:
    void hide();

  private:
    bim_declare_controls_struct(controls, m_controls, 2);
    const iscool::style::declaration& m_style_bounds;

    ax::Node& m_container;

    bim::axmol::input::touch_sink_handle m_touch_sink;
    bim::axmol::input::key_sink_handle m_key_sink;
    bim::axmol::input::tree m_inputs;
  };
}
