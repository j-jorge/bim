// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer_handle.hpp>

namespace bim::axmol::input
{
  class scroll_view_glue;
  using scroll_view_glue_handle = touch_observer_handle<scroll_view_glue>;
}
