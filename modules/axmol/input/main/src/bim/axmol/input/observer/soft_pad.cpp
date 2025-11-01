// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/soft_pad.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <bim/axmol/bounding_box_on_screen.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Node.h>
#include <axmol/base/Touch.h>

#include <numbers>

IMPLEMENT_SIGNAL(bim::axmol::input::soft_pad, changed, m_changed);

bim::axmol::input::soft_pad::soft_pad(const ax::Node& reference)
  : axmol_node_touch_observer(reference)
  , m_vertical(0)
  , m_horizontal(0)
  , m_enabled(true)
{}

bim::axmol::input::soft_pad::~soft_pad() = default;

void bim::axmol::input::soft_pad::enable(bool v)
{
  m_enabled = v;

  if (!m_enabled)
    {
      m_vertical = 0;
      m_horizontal = 0;
    }
}

bool bim::axmol::input::soft_pad::is_enabled() const
{
  return m_enabled;
}

std::int8_t bim::axmol::input::soft_pad::vertical() const
{
  return m_vertical;
}

std::int8_t bim::axmol::input::soft_pad::horizontal() const
{
  return m_horizontal;
}

void bim::axmol::input::soft_pad::reset()
{
  m_horizontal = 0;
  m_vertical = 0;
}

void bim::axmol::input::soft_pad::do_pressed(touch_event& touch)
{
  if (m_touch_id || should_ignore_touches())
    return;

  check_touch(touch);
}

void bim::axmol::input::soft_pad::do_moved(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  check_touch(touch);
}

void bim::axmol::input::soft_pad::check_touch(touch_event& touch)
{
  const ax::Rect box = bim::axmol::bounding_box_on_screen(m_node);
  const ax::Vec2 size = box.size;
  const ax::Vec2 half_size = size / 2;

  int8_t vertical = 0;
  int8_t horizontal = 0;

  constexpr float min_range_squared = 1.f / 25;
  constexpr float angle_split = std::numbers::sqrt2_v<float> / 2;

  if (!touch.is_available() || !contains_touch(touch))
    return;

  const ax::Vec2 p =
      (touch.get()->getLocation() - box.origin - half_size) / half_size;

  if (p.getLengthSq() <= min_range_squared)
    return;

  touch.consume();
  m_touch_id = touch.get()->getID();

  const ax::Vec2 a = p.getNormalized();

  if ((a.y >= -angle_split) && (a.y <= angle_split))
    {
      if (a.x >= 0)
        horizontal = 1;
      else
        horizontal = -1;
    }

  if ((a.x >= -angle_split) && (a.x <= angle_split))
    {
      if (a.y >= 0)
        vertical = 1;
      else
        vertical = -1;
    }

  if ((vertical != m_vertical) || (horizontal != m_horizontal))
    {
      m_vertical = vertical;
      m_horizontal = horizontal;
      m_changed();
    }
}

void bim::axmol::input::soft_pad::do_released(touch_event& touch)
{
  if (!m_touch_id || should_ignore_touches())
    return;

  if (touch.get()->getID() == *m_touch_id)
    {
      if (touch.is_available())
        touch.consume();

      m_touch_id = std::nullopt;
      m_horizontal = 0;
      m_vertical = 0;
      m_changed();
    }
}

void bim::axmol::input::soft_pad::do_cancelled(touch_event& touch)
{
  do_released(touch);
}

void bim::axmol::input::soft_pad::do_unplugged()
{
  m_touch_id = std::nullopt;
  m_horizontal = 0;
  m_vertical = 0;
}

bool bim::axmol::input::soft_pad::should_ignore_touches() const
{
  return !m_enabled || axmol_node_touch_observer::should_ignore_touches();
}
