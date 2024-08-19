// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/backend_event_view.hpp>
#include <bim/axmol/input/key_event.hpp>

namespace bim::axmol::input
{
  using key_event_view = backend_event_view<key_event>;
}
