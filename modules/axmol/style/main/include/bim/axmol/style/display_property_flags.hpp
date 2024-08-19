// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/style/display_property_flags_fwd.hpp>

namespace bim::axmol::style
{
  enum class display_property_flags : std::uint16_t
  {
    anchor_point_x = (1 << 0),
    anchor_point_y = (1 << 1),
    rotation = (1 << 2),
    opacity = (1 << 3),

    z_order = (1 << 4),
    cascade_opacity = (1 << 5),
    cascade_color = (1 << 6),
    visible = (1 << 7),
    color = (1 << 8),
  };
}
