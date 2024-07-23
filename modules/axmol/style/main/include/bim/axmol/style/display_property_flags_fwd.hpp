#pragma once

#include <cstdint>

namespace bim::axmol::style
{
  enum class display_property_flags : std::uint16_t;

  display_property_flags operator&(display_property_flags lhs,
                                   display_property_flags rhs);
  display_property_flags operator|(display_property_flags lhs,
                                   display_property_flags rhs);
  display_property_flags& operator|=(display_property_flags& lhs,
                                     display_property_flags rhs);
}
