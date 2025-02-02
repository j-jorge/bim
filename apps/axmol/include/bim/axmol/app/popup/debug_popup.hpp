// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/tree.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/context.hpp>

#include <memory>

namespace iscool::preferences
{
  class local_preferences;
}

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
  class popup;

  class debug_popup
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((main_scene*)(main_scene))                            //
            ((iscool::preferences::local_preferences*)(local_preferences))),
        ic_context_no_properties);

  public:
    debug_popup(const context& context,
                const iscool::style::declaration& style);
    ~debug_popup();

    void show();

  private:
    void add_feature_item(std::string_view label,
                          bim::game::feature_flags flag);

    void add_title(std::string_view label);
    void add_item(const bim::axmol::widget::named_node_group& nodes,
                  const iscool::style::declaration& bounds);
    void add_text_item(std::string_view label, std::string_view value);
    void add_toggle_item(std::string_view label, bool state,
                         std::function<bool()> do_toggle);

  private:
    bim_declare_controls_struct(controls, m_controls, 2);
    const iscool::style::declaration& m_style_bounds;

    const iscool::style::declaration& m_list_item_container_style;

    const iscool::style::declaration& m_title_item_controls;
    const iscool::style::declaration& m_title_item_bounds;
    const iscool::style::declaration& m_text_item_controls;
    const iscool::style::declaration& m_text_item_bounds;
    const iscool::style::declaration& m_toggle_item_controls;
    const iscool::style::declaration& m_toggle_item_bounds;

    std::unique_ptr<popup> m_popup;

    bim::axmol::input::tree m_inputs;
  };
}
