#include <bim/axmol/input/key_observer.hpp>

bim::axmol::input::key_observer::key_observer() = default;
bim::axmol::input::key_observer::~key_observer() = default;

void bim::axmol::input::key_observer::pressed(const key_event_view& key)
{
  do_pressed(key);
}

void bim::axmol::input::key_observer::released(const key_event_view& key)
{
  do_released(key);
}

void bim::axmol::input::key_observer::do_pressed(const key_event_view& key)
{}

void bim::axmol::input::key_observer::do_released(const key_event_view& key)
{}
