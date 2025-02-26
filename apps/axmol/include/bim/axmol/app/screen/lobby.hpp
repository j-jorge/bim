// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/observer/tap_observer_handle.hpp>
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

namespace bim::axmol::app
{
  class debug_popup;
  class main_scene;
  class settings_popup;

  class lobby
  {
    DECLARE_VOID_SIGNAL(play, m_play)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((bim::net::session_handler*)(session_handler))                //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::social::service*)(social))                           //
            ((iscool::system::haptic_feedback*)(haptic_feedback))          //
            ((bool)(enable_debug))),
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

    void increment_debug_activator_counter();
    void enable_debug();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 4);

    std::unique_ptr<settings_popup> m_settings;
    std::unique_ptr<debug_popup> m_debug;

    iscool::signals::scoped_connection m_session_connection;

    bim::axmol::input::tap_observer_handle m_debug_tap;
    std::uint8_t m_debug_activator_counter;
  };
}
