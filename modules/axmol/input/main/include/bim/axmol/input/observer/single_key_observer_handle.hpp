#pragma once

#include <bim/axmol/input/key_observer_handle.hpp>

namespace bim::axmol::input
{
  class single_key_observer;
  using single_key_observer_handle = key_observer_handle<single_key_observer>;
}
