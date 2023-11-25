#pragma once

#include <bim/axmol/input/backend_event.hpp>

#include <axmol/base/EventKeyboard.h>

namespace bim::axmol::input
{
  using key_event = backend_event<ax::EventKeyboard::KeyCode>;
}
