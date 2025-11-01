// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/touch_observer.hpp>

bim::axmol::input::touch_observer::touch_observer() = default;
bim::axmol::input::touch_observer::~touch_observer() = default;

bool bim::axmol::input::touch_observer::may_process(touch_event& touch) const
{
  return do_may_process(touch);
}

void bim::axmol::input::touch_observer::pressed(touch_event& touch)
{
  do_pressed(touch);
}

void bim::axmol::input::touch_observer::moved(touch_event& touch)
{
  do_moved(touch);
}

void bim::axmol::input::touch_observer::released(touch_event& touch)
{
  do_released(touch);
}

void bim::axmol::input::touch_observer::cancelled(touch_event& touch)
{
  do_cancelled(touch);
}

void bim::axmol::input::touch_observer::unplugged()
{
  do_unplugged();
}

bool bim::axmol::input::touch_observer::do_may_process(
    touch_event& touch) const
{
  return true;
}
