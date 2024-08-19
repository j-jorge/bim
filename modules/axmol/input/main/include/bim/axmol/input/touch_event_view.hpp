// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/backend_event_view.hpp>
#include <bim/axmol/input/touch_event.hpp>

namespace bim::axmol::input
{
  using touch_event_view = backend_event_view<touch_event>;
}
