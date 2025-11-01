// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/tap_observer.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/base/Touch.h>

IMPLEMENT_SIGNAL(bim::axmol::input::tap_observer, enter, m_enter);
IMPLEMENT_SIGNAL(bim::axmol::input::tap_observer, leave, m_leave);
IMPLEMENT_SIGNAL(bim::axmol::input::tap_observer, release, m_release);

bim::axmol::input::tap_observer::tap_observer(const ax::Node& reference)
  : axmol_node_touch_observer(reference)
  , m_cooldown_end_date(0)
  , m_enabled(true)
  , m_cancel_on_swipe(true)
{}

bim::axmol::input::tap_observer::~tap_observer() = default;

void bim::axmol::input::tap_observer::enable(bool v)
{
  m_enabled = v;
}

bool bim::axmol::input::tap_observer::is_enabled() const
{
  return m_enabled;
}

void bim::axmol::input::tap_observer::cancel_on_swipe(bool v)
{
  m_cancel_on_swipe = v;
}

void bim::axmol::input::tap_observer::do_pressed(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  const bool was_pressed = is_pressed();

  update_touch_position(touch);

  if (!was_pressed && is_pressed())
    m_enter();
}

void bim::axmol::input::tap_observer::do_moved(touch_event& touch)
{
  if (!is_pressed() || should_ignore_touches() || !m_cancel_on_swipe)
    return;

  // If the move is larger than this threshold then we consider that the action
  // is not a tap, but maybe rather a swipe.
  constexpr float threshold = 16;

  const int id = touch.get()->getID();
  const auto it = m_touch_id_pressed_position.find(id);

  if (it != m_touch_id_pressed_position.end())
    {
      const ax::Vec2 offset = touch.get()->getLocation() - it->second;

      if ((std::abs(offset.x) >= threshold)
          || (std::abs(offset.y) >= threshold))
        m_touch_id_pressed_position.erase(it);
    }

  if (!is_pressed())
    m_leave();
}

void bim::axmol::input::tap_observer::do_released(touch_event& touch)
{
  if (!is_pressed() || should_ignore_touches())
    return;

  const bool consumed = consume_known_touch(touch);

  if (is_pressed())
    return;

  disable_temporarily();

  if (consumed)
    m_release();
  else
    m_leave();
}

void bim::axmol::input::tap_observer::do_cancelled(touch_event& touch)
{
  if (!is_pressed() || should_ignore_touches())
    return;

  consume_known_touch(touch);
  disable_temporarily();

  if (!is_pressed())
    m_leave();
}

void bim::axmol::input::tap_observer::do_unplugged()
{
  m_touch_id_pressed_position.clear();
  m_cooldown_end_date = {};
}

bool bim::axmol::input::tap_observer::should_ignore_touches() const
{
  return !m_enabled || is_temporarily_disabled()
         || axmol_node_touch_observer::should_ignore_touches();
}

bool bim::axmol::input::tap_observer::is_pressed() const
{
  return !m_touch_id_pressed_position.empty();
}

void bim::axmol::input::tap_observer::update_touch_position(touch_event& touch)
{
  const int id = touch.get()->getID();

  if (touch.is_available() && contains_touch(touch))
    {
      touch.consume();
      m_touch_id_pressed_position[id] = touch.get()->getLocation();
    }
  else
    m_touch_id_pressed_position.erase(id);
}

bool bim::axmol::input::tap_observer::consume_known_touch(touch_event& touch)
{
  if (m_touch_id_pressed_position.erase(touch.get()->getID()) != 0)
    if (touch.is_available())
      {
        touch.consume();
        return true;
      }

  return false;
}

void bim::axmol::input::tap_observer::disable_temporarily()
{
  constexpr std::chrono::milliseconds cooldown(100);

  m_cooldown_end_date =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
      + cooldown;
}

bool bim::axmol::input::tap_observer::is_temporarily_disabled() const
{
  return std::chrono::steady_clock::now().time_since_epoch()
         <= m_cooldown_end_date;
}
