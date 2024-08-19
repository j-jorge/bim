// SPDX-License-Identifier: AGPL-3.0-only
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
