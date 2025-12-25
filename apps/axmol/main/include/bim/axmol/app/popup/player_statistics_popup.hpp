// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/observer/touch_anywhere_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

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
  class application_event_dispatcher;
  class main_scene;
  class popup;

  class player_statistics_popup
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context*)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((application_event_dispatcher*)(event_dispatcher))            //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ),
        ic_context_no_properties);

  public:
    player_statistics_popup(const context& context,
                            const iscool::style::declaration& style);
    ~player_statistics_popup();

    void show();

  private:
    bim::axmol::input::single_key_observer_handle m_escape;
    bim::axmol::input::touch_anywhere_handle m_tap;

    bim_declare_controls_struct(controls, m_controls, 6);
    const iscool::style::declaration& m_style_bounds;

    std::unique_ptr<popup> m_popup;

    bim::axmol::input::tree m_inputs;
  };
}
