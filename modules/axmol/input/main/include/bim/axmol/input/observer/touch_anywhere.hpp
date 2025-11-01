// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_event.hpp>
#include <bim/axmol/input/touch_observer.hpp>

#include <iscool/schedule/async_signal.hpp>

#include <unordered_set>

namespace bim::axmol::input
{
  class touch_anywhere : public touch_observer
  {
    // Use async signals to avoid jumping anywhere during the touch handling,
    // maybe breaking the currently working observer instance.
    DECLARE_ASYNC_VOID_SIGNAL(release, m_release)

  public:
    touch_anywhere();
    ~touch_anywhere();

    void enable(bool v);
    bool is_enabled() const;

  private:
    void do_pressed(touch_event& touch) override;
    void do_moved(touch_event& touch) override;
    void do_released(touch_event& touch) override;
    void do_cancelled(touch_event& touch) override;

    void do_unplugged() override;

    bool should_ignore_touches() const;
    bool is_pressed() const;

    void update_touch_position(touch_event& touch);
    bool consume_known_touches(touch_event& touch);

    void disable_temporarily();
    bool is_temporarily_disabled() const;

  private:
    std::unordered_set<int> m_pressed_touches;

    bool m_enabled;
  };
}
