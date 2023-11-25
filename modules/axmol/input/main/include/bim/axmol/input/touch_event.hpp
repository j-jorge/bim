#pragma once

#include <bim/axmol/input/backend_event.hpp>

namespace ax
{
  class Touch;
}

namespace bim::axmol::input
{
  using touch_event = backend_event<ax::Touch*>;
}
