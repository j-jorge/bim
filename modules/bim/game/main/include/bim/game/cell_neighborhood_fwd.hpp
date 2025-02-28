// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  enum class cell_neighborhood : std::uint8_t;

  cell_neighborhood operator|(cell_neighborhood lhs, cell_neighborhood rhs);
  cell_neighborhood& operator|=(cell_neighborhood& lhs, cell_neighborhood rhs);

  cell_neighborhood operator&(cell_neighborhood lhs, cell_neighborhood rhs);
  cell_neighborhood& operator&=(cell_neighborhood& lhs, cell_neighborhood rhs);

  cell_neighborhood operator>>(cell_neighborhood lhs, int rhs);
  cell_neighborhood& operator>>=(cell_neighborhood& lhs, int rhs);

  cell_neighborhood operator<<(cell_neighborhood lhs, int rhs);
  cell_neighborhood& operator<<=(cell_neighborhood& lhs, int rhs);

  cell_neighborhood operator~(cell_neighborhood lhs);

  bool operator!(cell_neighborhood lhs);

  constexpr int cell_neighborhood_layout_count = 256;
}
