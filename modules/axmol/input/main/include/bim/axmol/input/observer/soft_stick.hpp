// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/observer/axmol_node_touch_observer.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <iscool/schedule/async_signal.hpp>

#include <axmol/math/Vec2.h>

#include <optional>

namespace bim::axmol::input
{
  /**
   * Like a joystick/analog stick, but in software.
   */
  class soft_stick : public axmol_node_touch_observer
  {
    DECLARE_ASYNC_SIGNAL(void(ax::Vec2), move, m_move)
    DECLARE_ASYNC_VOID_SIGNAL(up, m_up)

  public:
    soft_stick(const ax::Node& reference, const ax::Node& stick);
    ~soft_stick();

    void enable(bool v);
    bool is_enabled() const;

    const ax::Vec2& drag() const;

    void reset();

  private:
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;
    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;

    bool should_ignore_touches() const;

    void constraint_drag(ax::Vec2 touch_location);

  private:
    const ax::Node& m_stick;
    std::optional<int> m_touch_id;
    ax::Vec2 m_drag;
    bool m_enabled;
  };
}
