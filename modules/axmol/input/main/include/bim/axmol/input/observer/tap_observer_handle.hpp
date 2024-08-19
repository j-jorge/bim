// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer_handle.hpp>

namespace bim::axmol::input
{
  class tap_observer;
  using tap_observer_handle = touch_observer_handle<tap_observer>;
}
