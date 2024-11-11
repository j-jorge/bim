// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/observer/axmol_node_touch_observer.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <iscool/schedule/async_signal.hpp>

#include <axmol/math/Vec2.h>

#include <chrono>
#include <unordered_map>

namespace bim::axmol::input
{
  class tap_observer : public axmol_node_touch_observer
  {
    // Use async signals to avoid jumping anywhere during the touch handling,
    // maybe breaking the currently working observer instance.
    DECLARE_ASYNC_VOID_SIGNAL(enter, m_enter)
    DECLARE_ASYNC_VOID_SIGNAL(leave, m_leave)
    DECLARE_ASYNC_VOID_SIGNAL(release, m_release)

  public:
    explicit tap_observer(const ax::Node& reference);
    ~tap_observer();

    void enable(bool v);
    bool is_enabled() const;

    void cancel_on_swipe(bool v);

  private:
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;
    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;

    bool should_ignore_touches() const;
    bool is_pressed() const;

    void update_touch_position(touch_event& touch);
    bool consume_known_touches(const touch_event_view& touches);

    void disable_temporarily();
    bool is_temporarily_disabled() const;

  private:
    std::unordered_map<int, ax::Vec2> m_touch_id_pressed_position;

    /**
     * We disable the observer for a short time when a touch is processed, in
     * order to avoid spurious events, spam, etc.
     */
    std::chrono::milliseconds m_cooldown_end_date;
    bool m_enabled;

    /**
     * When a widget is in a scrollable list the user may start a drag on the
     * widget, in which case we don't want to trigger a tap when the finger is
     * released on the widget at the end of the scroll. Enable this flag to
     * prevent tap detection in this case.
     */
    bool m_cancel_on_swipe;
  };
}
