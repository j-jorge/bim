// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer_handle.hpp>

namespace bim::axmol::input
{
  class touch_sink;
  using touch_sink_handle = touch_observer_handle<touch_sink>;
}
