// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <memory>

namespace bim::axmol::input
{
  class touch_observer;
  using touch_observer_pointer = std::weak_ptr<touch_observer>;
}
