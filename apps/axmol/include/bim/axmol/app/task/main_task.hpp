// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/session_handler.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <memory>

namespace iscool
{
  namespace audio
  {
    class mixer;
  }

  namespace preferences
  {
    class local_preferences;
  }

  namespace system
  {
    class haptic_feedback;
  }
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::axmol::app
{
  class main_scene;
  class scene_lock;
  class screen_wheel;

  class main_task
  {
    DECLARE_VOID_SIGNAL(end, m_end)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((scene_lock*)(scene_lock))                                    //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::system::haptic_feedback*)(haptic_feedback))),
        ic_context_declare_properties(
            ((bim::net::session_handler*)(session_handler))));

  public:
    explicit main_task(context context);
    ~main_task();

    void start();

  private:
    bim::net::session_handler m_session_handler;
    std::unique_ptr<screen_wheel> m_screen_wheel;
  };
}
