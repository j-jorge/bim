// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  enum class cell_edge : std::uint8_t;

  cell_edge operator|(cell_edge lhs, cell_edge rhs);
  cell_edge& operator|=(cell_edge& lhs, cell_edge rhs);

  cell_edge operator&(cell_edge lhs, cell_edge rhs);
  cell_edge& operator&=(cell_edge& lhs, cell_edge rhs);

  cell_edge operator~(cell_edge lhs);

  bool operator!(cell_edge lhs);

  cell_edge horizontal_flip(cell_edge e);
  cell_edge vertical_flip(cell_edge e);

  constexpr int cell_edge_count = 4;
}
