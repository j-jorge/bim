#pragma once

#include <cstdint>

namespace bim::axmol::style
{
  enum class bounds_property_flags : std::uint16_t;

  bounds_property_flags operator&(bounds_property_flags lhs,
                                  bounds_property_flags rhs);
  bounds_property_flags operator|(bounds_property_flags lhs,
                                  bounds_property_flags rhs);
  bounds_property_flags& operator|=(bounds_property_flags& lhs,
                                    bounds_property_flags rhs);
}
