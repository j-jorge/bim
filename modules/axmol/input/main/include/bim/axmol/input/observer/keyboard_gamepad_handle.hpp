// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/key_observer_handle.hpp>

namespace bim::axmol::input
{
  class keyboard_gamepad;
  using keyboard_gamepad_handle = key_observer_handle<keyboard_gamepad>;
}
