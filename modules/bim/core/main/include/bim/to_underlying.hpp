// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <type_traits>

namespace bim
{
  template <typename T>
    requires requires { std::is_enum_v<T>; }
  std::underlying_type_t<T> to_underlying(T v)
  {
    return std::underlying_type_t<T>(v);
  }
}
