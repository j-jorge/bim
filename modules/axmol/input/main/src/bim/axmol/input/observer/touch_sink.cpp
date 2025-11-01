// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/observer/touch_sink.hpp>

#include <bim/axmol/input/touch_event.hpp>

void bim::axmol::input::touch_sink::do_pressed(touch_event& touch)
{
  if (touch.is_available())
    touch.consume();
}

void bim::axmol::input::touch_sink::do_moved(touch_event& touch)
{
  if (touch.is_available())
    touch.consume();
}

void bim::axmol::input::touch_sink::do_released(touch_event& touch)
{
  if (touch.is_available())
    touch.consume();
}

void bim::axmol::input::touch_sink::do_cancelled(touch_event& touch)
{
  if (touch.is_available())
    touch.consume();
}

void bim::axmol::input::touch_sink::do_unplugged()
{}
