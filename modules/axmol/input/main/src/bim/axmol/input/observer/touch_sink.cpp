#include <bim/axmol/input/observer/touch_sink.hpp>

#include <bim/axmol/input/touch_event_view.hpp>

void bim::axmol::input::touch_sink::do_pressed(const touch_event_view& touches)
{
  touches.consume_all();
}

void bim::axmol::input::touch_sink::do_moved(const touch_event_view& touches)
{
  touches.consume_all();
}

void bim::axmol::input::touch_sink::do_released(
    const touch_event_view& touches)
{
  touches.consume_all();
}

void bim::axmol::input::touch_sink::do_cancelled(
    const touch_event_view& touches)
{
  touches.consume_all();
}
