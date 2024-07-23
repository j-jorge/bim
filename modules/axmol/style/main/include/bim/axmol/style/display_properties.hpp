#pragma once

#include <bim/axmol/style/display_property_flags_fwd.hpp>

#include <axmol/base/Types.h>

namespace bim::axmol::style
{
  class display_properties
  {
  public:
    display_property_flags flags;

    ax::Color3B color;

    float anchor_point_x;
    float anchor_point_y;
    float rotation;
    float opacity;

    short z_order;
    bool cascade_opacity;
    bool cascade_color;
    bool visible;
  };
}
