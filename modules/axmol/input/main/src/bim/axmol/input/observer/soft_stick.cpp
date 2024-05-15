#include <bim/axmol/input/observer/soft_stick.hpp>

#include <bim/axmol/input/touch_event_view.hpp>

#include <bim/axmol/bounding_box_on_screen.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Node.h>
#include <axmol/base/Touch.h>

IMPLEMENT_SIGNAL(bim::axmol::input::soft_stick, changed, m_changed);

bim::axmol::input::soft_stick::soft_stick(const ax::Node& reference)
  : axmol_node_touch_observer(reference)
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

void bim::axmol::input::soft_stick::do_pressed(const touch_event_view& touches)
{
  if (should_ignore_touches())
    return;

  if (m_touch_id)
    return;

  assert(!std::empty(touches));

  touch_event& touch = *std::begin(touches);

  if (!touch.is_available() || !contains_touch(touch))
    return;

  touch.consume();

  m_touch_id = touch.get()->getID();

  const ax::Rect node_box_on_screen =
      bim::axmol::bounding_box_on_screen(m_node);
  const ax::Vec2 half_size = node_box_on_screen.size / 2;
  const ax::Vec2 quarter_size = node_box_on_screen.size / 4;
  const ax::Vec2 node_center_on_screen = node_box_on_screen.origin + half_size;
  const ax::Vec2 touch_position = touch.get()->getLocation();
  const ax::Vec2 distance_to_center = touch_position - node_center_on_screen;

  if ((std::abs(distance_to_center.x) < quarter_size.x)
      && (std::abs(distance_to_center.y) < quarter_size.y))
    // The touch is near the center, let's assume the player wants a smooth
    // movement.
    m_origin = touch_position;
  else
    {
      // Far from the center, the player wants to move in this direction.
      m_origin = node_center_on_screen;
      constraint_drag(touch_position, half_size);
      m_changed();
    }
}

void bim::axmol::input::soft_stick::do_moved(const touch_event_view& touches)
{
  if (!m_touch_id || should_ignore_touches())
    return;

  bool dispatch_changed = false;

  for (touch_event& touch : touches)
    if (touch.is_available() && (touch.get()->getID() == *m_touch_id))
      {
        touch.consume();

        const ax::Vec2 size = bim::axmol::bounding_box_on_screen(m_node).size;
        const ax::Vec2 half_size = size / 2;

        constraint_drag(touch.get()->getLocation(), half_size);
        dispatch_changed = true;

        break;
      }

  if (dispatch_changed)
    m_changed();
}

void bim::axmol::input::soft_stick::do_released(
    const touch_event_view& touches)
{
  do_cancelled(touches);
}

void bim::axmol::input::soft_stick::do_cancelled(
    const touch_event_view& touches)
{
  if (!m_touch_id || should_ignore_touches())
    return;

  for (touch_event& touch : touches)
    if (touch.get()->getID() == *m_touch_id)
      {
        if (touch.is_available())
          touch.consume();

        m_touch_id = std::nullopt;
        m_drag = {};
        m_changed();

        break;
      }
}

bool bim::axmol::input::soft_stick::should_ignore_touches() const
{
  return !m_enabled || axmol_node_touch_observer::should_ignore_touches();
}

void bim::axmol::input::soft_stick::constraint_drag(ax::Vec2 touch_location,
                                                    ax::Vec2 scale)
{
  m_drag = touch_location - m_origin;

  // Drag is proportional to the reference node, but the stick should not go to
  // the edges (otherwise it's ugly).
  m_drag = m_drag / scale;

  constexpr float max_length = 2.f / 3;

  if (m_drag.lengthSquared() >= max_length * max_length)
    {
      m_drag.normalize();
      m_drag.scale(max_length);
    }
}
