// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/key_observer_handle.hpp>

namespace bim::axmol::input
{
  class key_sink;
  using key_sink_handle = key_observer_handle<key_sink>;
}
