// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer_handle.hpp>

namespace bim::axmol::input
{
  class axmol_node_touch_observer;
  using axmol_node_touch_observer_handle =
      touch_observer_handle<axmol_node_touch_observer>;
}
