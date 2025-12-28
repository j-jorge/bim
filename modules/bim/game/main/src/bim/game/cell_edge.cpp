// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/cell_edge.hpp>

#include <type_traits>

bim::game::cell_edge bim::game::operator|(cell_edge lhs, cell_edge rhs)
{
  return cell_edge((std::underlying_type_t<cell_edge>)lhs
                   | (std::underlying_type_t<cell_edge>)rhs);
}

bim::game::cell_edge& bim::game::operator|=(cell_edge& lhs, cell_edge rhs)
{
  lhs = lhs | rhs;
  return lhs;
}

bim::game::cell_edge bim::game::operator&(cell_edge lhs, cell_edge rhs)
{
  return cell_edge((std::underlying_type_t<cell_edge>)lhs
                   & (std::underlying_type_t<cell_edge>)rhs);
}

bim::game::cell_edge& bim::game::operator&=(cell_edge& lhs, cell_edge rhs)
{
  lhs = lhs & rhs;
  return lhs;
}

bim::game::cell_edge bim::game::operator~(cell_edge lhs)
{
  return cell_edge(~(std::underlying_type_t<cell_edge>)lhs) & cell_edge::all;
}

bool bim::game::operator!(cell_edge lhs)
{
  return (std::underlying_type_t<cell_edge>)lhs == 0;
}

bim::game::cell_edge bim::game::horizontal_flip(cell_edge e)
{
  cell_edge r = e & ~(cell_edge::left | cell_edge::right);

  if (!!(e & cell_edge::left))
    r |= cell_edge::right;

  if (!!(e & cell_edge::right))
    r |= cell_edge::left;

  return r;
}

bim::game::cell_edge bim::game::vertical_flip(cell_edge e)
{
  cell_edge r = e & ~(cell_edge::up | cell_edge::down);

  if (!!(e & cell_edge::up))
    r |= cell_edge::down;

  if (!!(e & cell_edge::down))
    r |= cell_edge::up;

  return r;
}
