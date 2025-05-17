// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <array>

namespace bim
{
  template <typename E, typename T, int M = sizeof(E) * 8 + 1>
  class bit_map
  {
  public:
    const T& operator[](E v) const;
    T& operator[](E v);

  private:
    std::array<T, M> m_data;
  };
}
