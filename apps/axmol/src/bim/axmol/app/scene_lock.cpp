#include <bim/axmol/app/scene_lock.hpp>

#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/signals/implement_signal.hpp>

ic_implement_state_monitor(bim::axmol::app::scene_lock, m_monitor, unlocked,
                           ((unlocked)((locking)(locked))) //
                           ((locking)((locked)))           //
                           ((locked)((unlocking)))         //
                           ((unlocking)((unlocked))));

bim::axmol::app::scene_lock::scene_lock(
    main_scene& scene, const iscool::style::declaration& style)
  : m_scene(scene)
{}

bim::axmol::app::scene_lock::~scene_lock() = default;

void bim::axmol::app::scene_lock::instant_lock()
{
  if (m_monitor->is_locked_state())
    return;

  m_monitor->set_locked_state();
}

void bim::axmol::app::scene_lock::lock()
{
  if (m_monitor->is_locked_state() || m_monitor->is_locking_state())
    return;
}

void bim::axmol::app::scene_lock::unlock()
{}
