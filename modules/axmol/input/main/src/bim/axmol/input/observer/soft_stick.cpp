// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/soft_stick.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <bim/axmol/bounding_box_on_screen.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Node.h>
#include <axmol/base/Touch.h>

IMPLEMENT_SIGNAL(bim::axmol::input::soft_stick, move, m_move);
IMPLEMENT_SIGNAL(bim::axmol::input::soft_stick, up, m_up);

bim::axmol::input::soft_stick::soft_stick(const ax::Node& reference,
                                          const ax::Node& stick)
  : axmol_node_touch_observer(reference)
  , m_stick(stick)
  , m_enabled(true)
{}

bim::axmol::input::soft_stick::~soft_stick() = default;

void bim::axmol::input::soft_stick::enable(bool v)
{
  m_enabled = v;

  if (!m_enabled)
    {
      m_touch_id = std::nullopt;
      m_drag = {};
    }
}

bool bim::axmol::input::soft_stick::is_enabled() const
{
  return m_enabled;
}

const ax::Vec2& bim::axmol::input::soft_stick::drag() const
{
  return m_drag;
}

void bim::axmol::input::soft_stick::reset()
{
  m_drag = {};
}

void bim::axmol::input::soft_stick::do_pressed(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  if (m_touch_id)
    return;

  if (!touch.is_available() || !contains_touch(touch))
    return;

  touch.consume();

  m_touch_id = touch.get()->getID();

  const ax::Vec2 touch_position = touch.get()->getLocation();

  m_drag = {};
  m_move(touch_position);
}

void bim::axmol::input::soft_stick::do_moved(touch_event& touch)
{
  if (!m_touch_id || should_ignore_touches())
    return;

  if (!touch.is_available() || (touch.get()->getID() != *m_touch_id))
    return;

  touch.consume();

  ax::Vec2 position = touch.get()->getLocation();
  constraint_drag(position);

  m_move(position);
}

void bim::axmol::input::soft_stick::do_released(touch_event& touch)
{
  do_cancelled(touch);
}

void bim::axmol::input::soft_stick::do_cancelled(touch_event& touch)
{
  if (!m_touch_id || should_ignore_touches())
    return;

  if (touch.get()->getID() != *m_touch_id)
    return;

  if (touch.is_available())
    touch.consume();

  m_touch_id = std::nullopt;
  m_drag = {};
  m_up();
}

void bim::axmol::input::soft_stick::do_unplugged()
{
  m_touch_id = std::nullopt;
  m_drag = {};
}

bool bim::axmol::input::soft_stick::should_ignore_touches() const
{
  return !m_enabled || axmol_node_touch_observer::should_ignore_touches();
}

void bim::axmol::input::soft_stick::constraint_drag(ax::Vec2 touch_location)
{
  const ax::Node& parent = *m_stick.getParent();
  const ax::Vec2 size = bim::axmol::bounding_box_on_screen(parent).size;
  const ax::Vec2 half_size = size / 2;

  const ax::Vec2 origin = parent.convertToWorldSpace(half_size);
  m_drag = (touch_location - origin) / half_size;

  if (m_drag.lengthSquared() >= 1)
    m_drag.normalize();
}
