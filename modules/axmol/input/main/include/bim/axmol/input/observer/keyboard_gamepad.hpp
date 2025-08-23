// SPDX License-Identifier: AGL-3.0-only

#pragma once

#include <bim/axmol/input/key_observer.hpp>

#include <iscool/schedule/async_signal.hpp>

namespace bim::axmol::input
{

  class keyboard_gamepad : public bim::axmol::input::key_observer
  {
  public:
    // Use async signals to avoid jumping anywhere during the handling of the
    // key, maybe breaking the currently working observer instance.
    DECLARE_ASYNC_VOID_SIGNAL(action, m_action)

  public:
    keyboard_gamepad(ax::EventKeyboard::KeyCode up_key,
                     ax::EventKeyboard::KeyCode down_key,
                     ax::EventKeyboard::KeyCode right_key,
                     ax::EventKeyboard::KeyCode left_key,
                     ax::EventKeyboard::KeyCode space_key);

    ~keyboard_gamepad();

    void enable(bool v);

    std::int8_t vertical() const;
    std::int8_t horizontal() const;

  private:
    void do_pressed(const key_event_view& keys) override;
    void do_released(const key_event_view& keys) override;

    void check_pressed(key_event& event);
    void check_released(key_event& event);
    void apply_directions();

  private:
    const ax::EventKeyboard::KeyCode m_up_key;
    const ax::EventKeyboard::KeyCode m_down_key;
    const ax::EventKeyboard::KeyCode m_right_key;
    const ax::EventKeyboard::KeyCode m_left_key;
    const ax::EventKeyboard::KeyCode m_space_key;

    bool m_up_is_pressed;
    bool m_down_is_pressed;
    bool m_right_is_pressed;
    bool m_left_is_pressed;
    bool m_space_is_pressed;

    bool m_enabled;
    std::int8_t m_vertical;
    std::int8_t m_horizontal;
  };

}
