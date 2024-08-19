// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/single_key_observer.hpp>

#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::axmol::input::single_key_observer, pressed, m_pressed);
IMPLEMENT_SIGNAL(bim::axmol::input::single_key_observer, released, m_released);

bim::axmol::input::single_key_observer::single_key_observer(
    ax::EventKeyboard::KeyCode key)
  : m_key(key)
  , m_enabled(true)
{}

bim::axmol::input::single_key_observer::~single_key_observer() = default;

void bim::axmol::input::single_key_observer::set_enabled(bool enabled)
{
  m_enabled = enabled;
}

void bim::axmol::input::single_key_observer::do_pressed(
    const key_event_view& keys)
{
  if (!m_enabled)
    return;

  if (consume_events(keys))
    m_pressed();
}

void bim::axmol::input::single_key_observer::do_released(
    const key_event_view& keys)
{
  if (!m_enabled)
    return;

  if (consume_events(keys))
    m_released();
}

bool bim::axmol::input::single_key_observer::consume_events(
    const key_event_view& keys) const
{
  bool result = false;

  for (key_event& event : keys)
    if (event.is_available() && (event.get() == m_key))
      {
        result = true;
        event.consume();
      }

  return result;
}
