// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/axmol_node_touch_observer.hpp>

#include <bim/axmol/input/touch_event.hpp>

#include <bim/axmol/bounding_box_on_screen.hpp>
#include <bim/axmol/displayed.hpp>

#include <axmol/2d/Node.h>

bim::axmol::input::axmol_node_touch_observer::axmol_node_touch_observer(
    const ax::Node& node)
  : m_node(node)
{}

bool bim::axmol::input::axmol_node_touch_observer::should_ignore_touches()
    const
{
  return !bim::axmol::displayed(m_node);
}

bool bim::axmol::input::axmol_node_touch_observer::contains_touch(
    touch_event& touch) const
{
  const ax::Rect bounding_box(bim::axmol::bounding_box_on_screen(m_node));

  return bounding_box.containsPoint(touch.get()->getLocation());
}

bool bim::axmol::input::axmol_node_touch_observer::do_may_process(
    touch_event& touch) const
{
  return contains_touch(touch);
}

void bim::axmol::input::axmol_node_touch_observer::do_pressed(touch_event&)
{}

void bim::axmol::input::axmol_node_touch_observer::do_moved(touch_event&)
{}

void bim::axmol::input::axmol_node_touch_observer::do_released(touch_event&)
{}

void bim::axmol::input::axmol_node_touch_observer::do_cancelled(touch_event&)
{}

void bim::axmol::input::axmol_node_touch_observer::do_unplugged()
{}
