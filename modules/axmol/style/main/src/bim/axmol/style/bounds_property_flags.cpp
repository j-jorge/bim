// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/style/bounds_property_flags_fwd.hpp>

bim::axmol::style::bounds_property_flags
bim::axmol::style::operator&(bounds_property_flags lhs,
                             bounds_property_flags rhs)
{
  return (bounds_property_flags)((unsigned)lhs & (unsigned)rhs);
}

bim::axmol::style::bounds_property_flags
bim::axmol::style::operator|(bounds_property_flags lhs,
                             bounds_property_flags rhs)
{
  return (bounds_property_flags)((unsigned)lhs | (unsigned)rhs);
}

bim::axmol::style::bounds_property_flags&
bim::axmol::style::operator|=(bounds_property_flags& lhs,
                              bounds_property_flags rhs)
{
  lhs = lhs | rhs;
  return lhs;
}
