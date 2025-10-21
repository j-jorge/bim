// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/touch_observer.hpp>

#include <bim/axmol/input/touch_event_view.hpp>

bim::axmol::input::touch_observer::touch_observer() = default;
bim::axmol::input::touch_observer::~touch_observer() = default;

bool bim::axmol::input::touch_observer::may_process(
    const touch_event_view& touches) const
{
  return do_may_process(touches);
}

void bim::axmol::input::touch_observer::pressed(
    const touch_event_view& touches)
{
  do_pressed(touches);
}

void bim::axmol::input::touch_observer::moved(const touch_event_view& touches)
{
  do_moved(touches);
}

void bim::axmol::input::touch_observer::released(
    const touch_event_view& touches)
{
  do_released(touches);
}

void bim::axmol::input::touch_observer::cancelled(
    const touch_event_view& touches)
{
  do_cancelled(touches);
}

bool bim::axmol::input::touch_observer::do_may_process(
    const touch_event_view& touches) const
{
  return true;
}
