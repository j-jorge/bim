// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <memory>

namespace iscool::audio
{
  class mixer;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::social
{
  class service;
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

namespace bim::app
{
  class analytics_service;
}

namespace bim::axmol::app
{
  class application_event_dispatcher;
  class language_popup;
  class main_scene;
  class popup;

  class settings_popup
  {
    DECLARE_VOID_SIGNAL(reset, m_reset)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((bim::app::analytics_service*)(analytics))                    //
            ((application_event_dispatcher*)(event_dispatcher))            //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::social::service*)(social))                           //
            ((iscool::system::haptic_feedback*)(haptic_feedback))),
        ic_context_no_properties);

  public:
    settings_popup(const context& context,
                   const iscool::style::declaration& style);
    ~settings_popup();

    void show();

  private:
    void set_direction_pad_display(bool pad_on_the_left);
    void set_stick_or_pad_display(bool use_stick);

    void set_music_preference();
    void set_sound_effects_preference();
    void set_vibrations_preference();
    void set_d_pad_position_preference();
    void set_d_pad_kind_preference();

    void show_language_selection();
    void open_bluesky();
    void open_github();
    void open_mail();
    void open_share();

  private:
    bim::axmol::input::single_key_observer_handle m_escape;

    bim_declare_controls_struct(controls, m_controls, 12);
    const iscool::style::declaration& m_style_bounds;

    const iscool::style::declaration& m_style_pad_on_the_left;
    const iscool::style::declaration& m_style_pad_on_the_right;

    const iscool::style::declaration& m_style_directions_stick;
    const iscool::style::declaration& m_style_directions_pad;

    std::unique_ptr<popup> m_popup;
    std::unique_ptr<language_popup> m_language_popup;

    bim::axmol::input::tree m_inputs;
  };
}
