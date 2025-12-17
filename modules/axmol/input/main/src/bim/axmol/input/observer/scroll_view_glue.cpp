// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/scroll_view_glue.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <bim/axmol/bounding_box_on_screen.hpp>
#include <bim/axmol/displayed.hpp>

#include <axmol/extensions/GUI/ScrollView/ScrollView.h>

bim::axmol::input::scroll_view_glue::scroll_view_glue(
    ax::extension::ScrollView& view)
  : m_view(view)
{
  m_view.setTouchEnabled(false);
}

bool bim::axmol::input::scroll_view_glue::should_ignore_touches() const
{
  return !bim::axmol::displayed(m_view);
}

bool bim::axmol::input::scroll_view_glue::contains_touch(
    touch_event& touch) const
{
  // The implementation of ax::ScrollView::getContentSize() changes the meaning
  // of the content size. While it is the visual bounds for all other nodes,
  // for this one it is the size of the actual scrollable content. The bounds
  // of the view is returned by getViewSize().

  const ax::Vec2 view_bottom_left = m_view.convertToWorldSpace(ax::Vec2(0, 0));
  const ax::Size view_size = m_view.getViewSize();
  const ax::Rect view_box(view_bottom_left, view_size);

  return view_box.containsPoint(touch.get()->getLocation());
}

void bim::axmol::input::scroll_view_glue::do_pressed(touch_event& touch)
{
  // The scroll is handled in do_moved, including the initial move. In
  // do_pressed we only handle the inputs to stop the scroll of the view,
  // i.e. when the user has "launched" the list and wants to stop it.
  if (!m_view.isScrolling() || should_ignore_touches())
    return;

  if (touch.is_available())
    {
      touch.consume();

      ax::Touch& t = *touch.get();
      const int id = t.getID();

      m_touch_initial_position[id] = t.getLocation();
      m_active_touch.insert(id);

      m_view.onTouchBegan(&t, nullptr);
    }
}

void bim::axmol::input::scroll_view_glue::do_moved(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  bool began = false;
  bool moved = false;
  categorize_moving_touch(touch, began, moved);

  if (began)
    m_view.onTouchBegan(touch.get(), nullptr);

  if (moved)
    m_view.onTouchMoved(touch.get(), nullptr);
}

void bim::axmol::input::scroll_view_glue::do_released(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  bool released = false;
  bool cancelled = false;
  categorize_released_touch(touch, released, cancelled);

  if (released)
    m_view.onTouchEnded(touch.get(), nullptr);

  if (cancelled)
    m_view.onTouchEnded(touch.get(), nullptr);
}

void bim::axmol::input::scroll_view_glue::do_cancelled(touch_event& touch)
{
  if (should_ignore_touches())
    return;

  bool cancelled = false;
  categorize_released_touch(touch, cancelled, cancelled);

  if (cancelled)
    m_view.onTouchCancelled(touch.get(), nullptr);
}

void bim::axmol::input::scroll_view_glue::do_unplugged()
{
  m_touch_initial_position.clear();
  m_active_touch.clear();
}

void bim::axmol::input::scroll_view_glue::categorize_moving_touch(
    touch_event& touch, bool& began, bool& moved)
{
  ax::Touch& t = *touch.get();
  const int id = t.getID();

  // Check if we are already tracking this touch, in which case it is thus
  // moving.
  if (m_active_touch.find(id) != m_active_touch.end())
    {
      if (touch.is_available())
        touch.consume();

      moved = true;
      return;
    }

  if (!touch.is_available())
    return;

  const std::unordered_map<int, ax::Vec2>::const_iterator it =
      m_touch_initial_position.find(id);

  // A new touch is not moving, by definition, and we keep its initial position
  // to check the distance trigger in the next iterations.
  if (it == m_touch_initial_position.end())
    {
      m_touch_initial_position[id] = t.getLocation();
      return;
    }

  constexpr float min_distance_for_active = 15;
  const float offset = it->second.y - t.getLocation().y;

  if ((std::abs(offset) >= min_distance_for_active) && contains_touch(touch))
    {
      touch.consume();
      m_active_touch.insert(id);
      began = true;
    }
}

void bim::axmol::input::scroll_view_glue::categorize_released_touch(
    touch_event& touch, bool& released, bool& cancelled)
{
  ax::Touch& t = *touch.get();
  const int id = t.getID();

  if (m_active_touch.erase(id) == 0)
    return;

  m_touch_initial_position.erase(id);

  if (touch.is_available())
    released = true;
  else
    cancelled = true;
}
