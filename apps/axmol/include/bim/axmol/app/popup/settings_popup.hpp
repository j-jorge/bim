// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>

#include <memory>

namespace iscool::audio
{
  class mixer;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace iscool::system
{
  class haptic_feedback;
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::axmol::app
{
  class main_scene;
  class popup;

  class settings_popup
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::system::haptic_feedback*)(haptic_feedback))),
        ic_context_no_properties);

  public:
    settings_popup(const context& context,
                   const iscool::style::declaration& style);
    ~settings_popup();

    void show();

  private:
    bim_declare_controls_struct(controls, m_controls, 4);
    const iscool::style::declaration& m_style_bounds;

    std::unique_ptr<popup> m_popup;

    bim::axmol::input::tree m_inputs;
  };
}