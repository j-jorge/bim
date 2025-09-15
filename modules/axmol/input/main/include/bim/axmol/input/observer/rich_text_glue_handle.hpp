// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer_handle.hpp>

namespace bim::axmol::input
{
  class rich_text_glue;
  using rich_text_glue_handle = touch_observer_handle<rich_text_glue>;
}
