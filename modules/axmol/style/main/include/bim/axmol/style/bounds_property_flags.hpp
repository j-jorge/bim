#pragma once

#include <bim/axmol/style/bounds_property_flags_fwd.hpp>

namespace bim::axmol::style
{
  enum class bounds_property_flags : std::uint16_t
  {
    anchor_x = (1 << 0),
    anchor_y = (1 << 1),
    anchor_in_reference_x = (1 << 2),
    anchor_in_reference_y = (1 << 3),

    percents_width = (1 << 4),
    percents_height = (1 << 5),
    width_ratio = (1 << 6),
    height_ratio = (1 << 7),
    keep_ratio = (1 << 8),

    scale_mode = (1 << 9),
    scale = (1 << 10),
    scale_constraint_max_percents_width = (1 << 11),
    scale_constraint_max_percents_height = (1 << 12),
  };
}
