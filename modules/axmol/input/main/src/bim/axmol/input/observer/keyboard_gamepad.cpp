// SPDX License-Identifier: AGL-3.0-only
#include <bim/axmol/input/observer/keyboard_gamepad.hpp>

#include <iscool/signals/implement_signal.hpp>
IMPLEMENT_SIGNAL(bim::axmol::input::keyboard_gamepad, action, m_action);

bim::axmol::input::keyboard_gamepad::keyboard_gamepad(
    ax::EventKeyboard::KeyCode up_key, ax::EventKeyboard::KeyCode down_key,
    ax::EventKeyboard::KeyCode right_key, ax::EventKeyboard::KeyCode left_key,
    ax::EventKeyboard::KeyCode space_key)
  : m_up_key(up_key)
  , m_down_key(down_key)
  , m_right_key(right_key)
  , m_left_key(left_key)
  , m_space_key(space_key)
  , m_up_is_pressed(false)
  , m_down_is_pressed(false)
  , m_right_is_pressed(false)
  , m_left_is_pressed(false)
  , m_space_is_pressed(false)
  , m_enabled(true)
  , m_vertical(0)
  , m_horizontal(0)
{}

bim::axmol::input::keyboard_gamepad::~keyboard_gamepad() = default;

void bim::axmol::input::keyboard_gamepad::enable(bool v)
{
  m_enabled = v;
}
std::int8_t bim::axmol::input::keyboard_gamepad::vertical() const
{
  return m_vertical;
}

std::int8_t bim::axmol::input::keyboard_gamepad::horizontal() const
{
  return m_horizontal;
}

void bim::axmol::input::keyboard_gamepad::do_pressed(
    const key_event_view& keys)
{
  if (!m_enabled)
    return;

  for (key_event& event : keys)
    if (event.is_available())
      {
        check_pressed(event);
      }

  apply_directions();
}

void bim::axmol::input::keyboard_gamepad::do_released(
    const key_event_view& keys)
{
  if (!m_enabled)
    return;

  for (key_event& event : keys)
    if (event.is_available())
      {
        check_released(event);
      }

  apply_directions();
}

void bim::axmol::input::keyboard_gamepad::check_pressed(key_event& event)
{
  if (event.get() == m_up_key)
    {
      m_up_is_pressed = true;
      event.consume();
    }
  else if (event.get() == m_down_key)
    {
      m_down_is_pressed = true;
      event.consume();
    }
  else if (event.get() == m_right_key)
    {
      m_right_is_pressed = true;
      event.consume();
    }
  else if (event.get() == m_left_key)
    {
      m_left_is_pressed = true;
      event.consume();
    }
  else if (event.get() == m_space_key)
    {
      m_action();
      m_space_is_pressed = true;
      event.consume();
    }
}

void bim::axmol::input::keyboard_gamepad::check_released(key_event& event)
{
  if (event.get() == m_up_key)
    {
      if (m_up_is_pressed)
        {
          m_up_is_pressed = false;
          event.consume();
        }
    }
  else if (event.get() == m_down_key)
    {
      if (m_down_is_pressed)
        {
          m_down_is_pressed = false;
          event.consume();
        }
    }
  else if (event.get() == m_right_key)
    {
      if (m_right_is_pressed)
        {
          m_right_is_pressed = false;
          event.consume();
        }
    }
  else if (event.get() == m_left_key)
    {
      if (m_left_is_pressed)
        {
          m_left_is_pressed = false;
          event.consume();
        }
    }
  else if (event.get() == m_space_key)
    {
      if (m_space_is_pressed)
        {
          m_space_is_pressed = false;
          event.consume();
        }
    }
}

void bim::axmol::input::keyboard_gamepad::apply_directions()
{
  m_horizontal = static_cast<std::int8_t>(m_right_is_pressed)
                 - static_cast<std::int8_t>(m_left_is_pressed);
  m_vertical = static_cast<std::int8_t>(m_up_is_pressed)
               - static_cast<std::int8_t>(m_down_is_pressed);
}