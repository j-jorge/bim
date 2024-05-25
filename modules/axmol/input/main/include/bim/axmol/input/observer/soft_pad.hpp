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

  private:
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;

    void check_touches(const touch_event_view& touches);

    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;

    bool should_ignore_touches() const;

  private:
    std::optional<int> m_touch_id;
    std::int8_t m_vertical;
    std::int8_t m_horizontal;
    bool m_enabled;
  };
}
