// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/cell_neighborhood.hpp>

#include <type_traits>

bim::game::cell_neighborhood bim::game::operator|(cell_neighborhood lhs,
                                                  cell_neighborhood rhs)
{
  return cell_neighborhood((std::underlying_type_t<cell_neighborhood>)lhs
                           | (std::underlying_type_t<cell_neighborhood>)rhs);
}

bim::game::cell_neighborhood& bim::game::operator|=(cell_neighborhood& lhs,
                                                    cell_neighborhood rhs)
{
  lhs = lhs | rhs;
  return lhs;
}

bim::game::cell_neighborhood bim::game::operator&(cell_neighborhood lhs,
                                                  cell_neighborhood rhs)
{
  return cell_neighborhood((std::underlying_type_t<cell_neighborhood>)lhs
                           & (std::underlying_type_t<cell_neighborhood>)rhs);
}

bim::game::cell_neighborhood& bim::game::operator&=(cell_neighborhood& lhs,
                                                    cell_neighborhood rhs)
{
  lhs = lhs & rhs;
  return lhs;
}

bim::game::cell_neighborhood bim::game::operator>>(cell_neighborhood lhs,
                                                   int rhs)
{
  return cell_neighborhood((std::underlying_type_t<cell_neighborhood>)lhs
                           >> rhs);
}

bim::game::cell_neighborhood& bim::game::operator>>=(cell_neighborhood& lhs,
                                                     int rhs)
{
  lhs = lhs >> rhs;
  return lhs;
}

bim::game::cell_neighborhood bim::game::operator<<(cell_neighborhood lhs,
                                                   int rhs)
{
  return cell_neighborhood((std::underlying_type_t<cell_neighborhood>)lhs
                           << rhs);
}

bim::game::cell_neighborhood& bim::game::operator<<=(cell_neighborhood& lhs,
                                                     int rhs)
{
  lhs = lhs << rhs;
  return lhs;
}

bim::game::cell_neighborhood bim::game::operator~(cell_neighborhood lhs)
{
  return cell_neighborhood(~(std::underlying_type_t<cell_neighborhood>)lhs);
}

bool bim::game::operator!(cell_neighborhood lhs)
{
  return (std::underlying_type_t<cell_neighborhood>)lhs == 0;
}
