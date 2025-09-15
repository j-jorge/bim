// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/observer/rich_text_glue_handle.hpp>
#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>
#include <iscool/language_name_fwd.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

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
  class analytics_service;
  class main_scene;
  class message_popup;
  class popup;

  class language_popup
  {
    DECLARE_VOID_SIGNAL(reset, m_reset)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((analytics_service*)(analytics))                              //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ),
        ic_context_no_properties);

  public:
    language_popup(const context& context,
                   const iscool::style::declaration& style);
    ~language_popup();

    void show();

  private:
    void confirm_language(iscool::language_name language);
    void switch_to_language(iscool::language_name language) const;

    void open_url(std::string_view url);

  private:
    bim_declare_controls_struct(controls, m_controls, 3);
    const iscool::style::declaration& m_style_bounds;

    std::unique_ptr<popup> m_popup;
    std::unique_ptr<message_popup> m_message;

    bim::axmol::input::single_key_observer_handle m_escape;
    bim::axmol::input::tree m_inputs;

    std::unique_ptr<bim::axmol::input::rich_text_glue_handle>
        m_add_language_inputs;

    iscool::signals::scoped_connection m_message_connexion;
  };
}
