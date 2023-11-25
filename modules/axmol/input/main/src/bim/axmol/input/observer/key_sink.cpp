#include <bim/axmol/input/observer/key_sink.hpp>

void bim::axmol::input::key_sink::do_pressed(const key_event_view& keys)
{
  keys.consume_all();
}

void bim::axmol::input::key_sink::do_released(const key_event_view& keys)
{
  keys.consume_all();
}
