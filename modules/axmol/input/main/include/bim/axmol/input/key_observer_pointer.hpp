// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <memory>

namespace bim::axmol::input
{
  class key_observer;
  using key_observer_pointer = std::weak_ptr<key_observer>;
}
