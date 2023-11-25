#include <bim/axmol/input/touch_observer.hpp>

#include <bim/axmol/input/touch_event_view.hpp>

bim::axmol::input::touch_observer::touch_observer() = default;
bim::axmol::input::touch_observer::~touch_observer() = default;

bool bim::axmol::input::touch_observer::is_relevant_to_pressed(
    const touch_event_view& touches) const
{
  return do_is_relevant_to_pressed(touches);
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

bool bim::axmol::input::touch_observer::do_is_relevant_to_pressed(
    const touch_event_view& touches) const
{
  return true;
}
