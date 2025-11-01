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
   * Like a joypad, but in software.
   */
  class soft_pad : public axmol_node_touch_observer
  {
    DECLARE_ASYNC_VOID_SIGNAL(changed, m_changed)

  public:
    explicit soft_pad(const ax::Node& reference);
    ~soft_pad();

    void enable(bool v);
    bool is_enabled() const;

    std::int8_t vertical() const;
    std::int8_t horizontal() const;

    void reset();

  private:
    void do_pressed(touch_event& touch) override;
    void do_moved(touch_event& touch) override;

    void check_touch(touch_event& touch);

    void do_released(touch_event& touch) override;
    void do_cancelled(touch_event& touch) override;

    void do_unplugged() override;

    bool should_ignore_touches() const;

  private:
    std::optional<int> m_touch_id;
    std::int8_t m_vertical;
    std::int8_t m_horizontal;
    bool m_enabled;
  };
}
