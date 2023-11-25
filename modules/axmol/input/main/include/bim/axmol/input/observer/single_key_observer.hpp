#pragma once

#include <bim/axmol/input/key_observer.hpp>

#include <iscool/schedule/async_signal.hpp>

namespace bim::axmol::input
{
  class single_key_observer : public bim::axmol::input::key_observer
  {
    // Use async signals to avoid jumping anywhere during the handling of the
    // key, maybe breaking the currently working observer instance.
    DECLARE_ASYNC_VOID_SIGNAL(pressed, m_pressed)
    DECLARE_ASYNC_VOID_SIGNAL(released, m_released)

  public:
    explicit single_key_observer(ax::EventKeyboard::KeyCode key);
    ~single_key_observer();

    void set_enabled(bool enabled);

  private:
    void do_pressed(const key_event_view& keys) override;
    void do_released(const key_event_view& keys) override;

    bool consume_events(const key_event_view& keys) const;

  private:
    const ax::EventKeyboard::KeyCode m_key;
    bool m_enabled;
  };
}
