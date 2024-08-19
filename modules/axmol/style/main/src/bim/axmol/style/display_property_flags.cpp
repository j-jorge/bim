// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/style/display_property_flags_fwd.hpp>

bim::axmol::style::display_property_flags
bim::axmol::style::operator&(display_property_flags lhs,
                             display_property_flags rhs)
{
  return (display_property_flags)((unsigned)lhs & (unsigned)rhs);
}

bim::axmol::style::display_property_flags
bim::axmol::style::operator|(display_property_flags lhs,
                             display_property_flags rhs)
{
  return (display_property_flags)((unsigned)lhs | (unsigned)rhs);
}

bim::axmol::style::display_property_flags&
bim::axmol::style::operator|=(display_property_flags& lhs,
                              display_property_flags rhs)
{
  lhs = lhs | rhs;
  return lhs;
}
