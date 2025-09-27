// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/style/bounds_property_flags_fwd.hpp>
#include <bim/axmol/style/scale_mode_fwd.hpp>

namespace bim::axmol::style
{
  class bounds_properties
  {
  public:
    bounds_property_flags flags;

    // position
    float anchor_x;
    float anchor_y;
    float anchor_in_reference_x;
    float anchor_in_reference_y;
    float offset_x;
    float offset_y;

    // size
    float percents_width;
    float percents_height;
    float ratio;
    float max_percents_size;
    bool keep_ratio;

    // scale
    bim::axmol::style::scale_mode scale_mode;
    float scale;
    float scale_constraint_max_percents_width;
    float scale_constraint_max_percents_height;
  };
}
