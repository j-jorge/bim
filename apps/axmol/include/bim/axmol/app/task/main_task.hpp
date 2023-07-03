#pragma once

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

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

namespace bim::axmol::app
{
  class main_scene;
  class scene_lock;

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
        ic_context_no_properties);

  public:
    explicit main_task(context context);

    void start();

  private:
    // ic_declare_state_monitor(m_monitor);
  };
}
