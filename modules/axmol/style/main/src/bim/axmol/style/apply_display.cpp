#include <bim/axmol/style/apply_display.hpp>

#include <bim/axmol/style/display_properties.hpp>
#include <bim/axmol/style/display_property_flags.hpp>

#include <axmol/2d/Node.h>

void bim::axmol::style::apply_display(const display_properties& display,
                                      ax::Node& node)
{
  if (bool(display.flags & display_property_flags::color))
    node.setColor(display.color);

  if (bool(display.flags & display_property_flags::anchor_point_x)
      || bool(display.flags & display_property_flags::anchor_point_y))
    {
      ax::Vec2 anchor_point = node.getAnchorPoint();

      if (bool(display.flags & display_property_flags::anchor_point_x))
        anchor_point.x = display.anchor_point_x;

      if (bool(display.flags & display_property_flags::anchor_point_y))
        anchor_point.y = display.anchor_point_y;

      node.setAnchorPoint(anchor_point);
    }

  if (bool(display.flags & display_property_flags::rotation))
    node.setRotation(display.rotation);

  if (bool(display.flags & display_property_flags::opacity))
    node.setOpacity(display.opacity * 255);

  if (bool(display.flags & display_property_flags::z_order))
    node.setLocalZOrder(display.z_order);

  if (bool(display.flags & display_property_flags::cascade_opacity))
    node.setCascadeOpacityEnabled(display.cascade_opacity);

  if (bool(display.flags & display_property_flags::cascade_color))
    node.setCascadeColorEnabled(display.cascade_color);

  if (bool(display.flags & display_property_flags::visible))
    node.setVisible(display.visible);
}
