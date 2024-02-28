#pragma once

#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class main_scene;

  class scene_lock
  {
    DECLARE_VOID_SIGNAL(unlocked, m_unlocked)

  public:
    scene_lock(main_scene& scene, const iscool::style::declaration& style);
    ~scene_lock();

    void instant_lock();
    void lock();
    void unlock();

  private:
    main_scene& m_scene;

    ic_declare_state_monitor(m_monitor);
  };
}
