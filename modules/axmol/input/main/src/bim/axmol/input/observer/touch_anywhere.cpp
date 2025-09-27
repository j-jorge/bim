// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/touch_anywhere.hpp>

#include <bim/axmol/input/touch_event_view.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/base/Touch.h>

IMPLEMENT_SIGNAL(bim::axmol::input::touch_anywhere, release, m_release);

bim::axmol::input::touch_anywhere::touch_anywhere()
  : m_enabled(true)
{}

bim::axmol::input::touch_anywhere::~touch_anywhere() = default;

void bim::axmol::input::touch_anywhere::enable(bool v)
{
  m_enabled = v;
}

bool bim::axmol::input::touch_anywhere::is_enabled() const
{
  return m_enabled;
}

void bim::axmol::input::touch_anywhere::do_pressed(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  for (touch_event& touch : touches)
    if (touch.is_available())
      {
        m_pressed_touches.insert(touch.get()->getID());
        touch.consume();
      }
}

void bim::axmol::input::touch_anywhere::do_moved(
    const touch_event_view& touches)
{}

void bim::axmol::input::touch_anywhere::do_released(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  if (m_pressed_touches.empty())
    return;

  for (touch_event& touch : touches)
    if (touch.is_available() && m_pressed_touches.erase(touch.get()->getID()))
      touch.consume();

  if (m_pressed_touches.empty())
    m_release();
}

void bim::axmol::input::touch_anywhere::do_cancelled(
    const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  if (m_pressed_touches.empty())
    return;

  for (touch_event& touch : touches)
    if (touch.is_available() && m_pressed_touches.erase(touch.get()->getID()))
      touch.consume();
}

bool bim::axmol::input::touch_anywhere::should_ignore_touches() const
{
  return !m_enabled;
}
