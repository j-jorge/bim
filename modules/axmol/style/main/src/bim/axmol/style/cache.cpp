// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/style/bounds_properties.hpp>
#include <bim/axmol/style/bounds_property_flags.hpp>
#include <bim/axmol/style/display_properties.hpp>
#include <bim/axmol/style/display_property_flags.hpp>
#include <bim/axmol/style/scale_mode.hpp>

#include <bim/axmol/colour_chart.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.impl.tpp>

static void fill_position(bim::axmol::style::bounds_properties& properties,
                          const iscool::style::declaration& style)
{
  if (const iscool::optional<float> anchor_x = style.get_number("anchor.x"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::anchor_x;
      properties.anchor_x = *anchor_x;
    }

  if (const iscool::optional<float> anchor_y = style.get_number("anchor.y"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::anchor_y;
      properties.anchor_y = *anchor_y;
    }

  if (const iscool::optional<float> anchor_in_reference_x =
          style.get_number("anchor.reference.x"))
    {
      properties.flags |=
          bim::axmol::style::bounds_property_flags::anchor_in_reference_x;
      properties.anchor_in_reference_x = *anchor_in_reference_x;
    }

  if (const iscool::optional<float> anchor_in_reference_y =
          style.get_number("anchor.reference.y"))
    {
      properties.flags |=
          bim::axmol::style::bounds_property_flags::anchor_in_reference_y;
      properties.anchor_in_reference_y = *anchor_in_reference_y;
    }

  if (const iscool::optional<float> offset_x = style.get_number("offset.x"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::offset_x;
      properties.offset_x = *offset_x;
    }

  if (const iscool::optional<float> offset_y = style.get_number("offset.y"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::offset_y;
      properties.offset_y = *offset_y;
    }
}

static void fill_size(bim::axmol::style::bounds_properties& properties,
                      const iscool::style::declaration& style)
{
  if (const iscool::optional<float> percents_width =
          style.get_number("width.percents"))
    {
      properties.flags |=
          bim::axmol::style::bounds_property_flags::percents_width;
      properties.percents_width = *percents_width;
    }

  if (const iscool::optional<float> percents_height =
          style.get_number("height.percents"))
    {
      properties.flags |=
          bim::axmol::style::bounds_property_flags::percents_height;
      properties.percents_height = *percents_height;
    }

  if (const iscool::optional<float> keep_ratio =
          style.get_boolean("keep-ratio"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::keep_ratio;
      properties.keep_ratio = *keep_ratio;
    }
  else if (const iscool::optional<float> width_ratio =
               style.get_number("width-ratio"))
    {
      properties.flags |=
          bim::axmol::style::bounds_property_flags::width_ratio;
      properties.ratio = *width_ratio;

      if (const iscool::optional<float> max_percents_size =
              style.get_number("width.max-percents"))
        {
          properties.flags |=
              bim::axmol::style::bounds_property_flags::max_percents_size;
          properties.max_percents_size = *max_percents_size;
        }
    }
  else if (const iscool::optional<float> height_ratio =
               style.get_number("height-ratio"))
    {
      properties.flags |=
          bim::axmol::style::bounds_property_flags::height_ratio;
      properties.ratio = *height_ratio;

      if (const iscool::optional<float> max_percents_size =
              style.get_number("height.max-percents"))
        {
          properties.flags |=
              bim::axmol::style::bounds_property_flags::max_percents_size;
          properties.max_percents_size = *max_percents_size;
        }
    }
}

static void fill_scale(bim::axmol::style::bounds_properties& properties,
                       const iscool::style::declaration& style)
{
  if (const iscool::optional<const std::string&> scale_mode =
          style.get_string("scale-mode"))
    {
      if (*scale_mode == "cover")
        {
          properties.flags |=
              bim::axmol::style::bounds_property_flags::scale_mode;
          properties.scale_mode = bim::axmol::style::scale_mode::cover;
        }
      else if (*scale_mode == "device")
        {
          properties.flags |=
              bim::axmol::style::bounds_property_flags::scale_mode;
          properties.scale_mode = bim::axmol::style::scale_mode::device;
        }
      else if (*scale_mode == "fit")
        {
          properties.flags |=
              bim::axmol::style::bounds_property_flags::scale_mode;
          properties.scale_mode = bim::axmol::style::scale_mode::fit;
        }
    }
  else if (const iscool::optional<float> scale = style.get_number("scale"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::scale;
      properties.scale = *scale;
      return;
    }

  const iscool::optional<const iscool::style::declaration&> scale_constraints =
      style.get_declaration("scale-constraints");

  if (!scale_constraints)
    return;

  if (const iscool::optional<float> max_percents_width_constraint =
          scale_constraints->get_number("max-percents-width"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::
          scale_constraint_max_percents_width;
      properties.scale_constraint_max_percents_width =
          *max_percents_width_constraint;
    }

  if (const iscool::optional<float> max_percents_height_constraint =
          scale_constraints->get_number("max-percents-height"))
    {
      properties.flags |= bim::axmol::style::bounds_property_flags::
          scale_constraint_max_percents_height;
      properties.scale_constraint_max_percents_height =
          *max_percents_height_constraint;
    }
}

bim::axmol::style::cache::cache(const bim::axmol::colour_chart& colour_chart)
  : m_colour_chart(colour_chart)
{}

bim::axmol::style::cache::~cache() = default;

const bim::axmol::style::bounds_properties&
bim::axmol::style::cache::get_bounds(const iscool::style::declaration& style)
{
  const bounds_map::const_iterator it = m_bounds.find(style.get_id());

  if (it != m_bounds.end())
    return it->second;

  bounds_properties& properties = m_bounds[style.get_id()];
  properties.flags = bounds_property_flags{};

  fill_position(properties, style);
  fill_size(properties, style);
  fill_scale(properties, style);

  return properties;
}

const bim::axmol::style::display_properties&
bim::axmol::style::cache::get_display(const iscool::style::declaration& style)
{
  const display_map::const_iterator it = m_display.find(style.get_id());

  if (it != m_display.end())
    return it->second;

  display_properties& properties = m_display[style.get_id()];
  properties.flags = display_property_flags{};

  if (const iscool::optional<float> anchor_point_x =
          style.get_number("anchor-point.x"))
    {
      properties.flags |=
          bim::axmol::style::display_property_flags::anchor_point_x;
      properties.anchor_point_x = *anchor_point_x;
    }

  if (const iscool::optional<float> anchor_point_y =
          style.get_number("anchor-point.y"))
    {
      properties.flags |=
          bim::axmol::style::display_property_flags::anchor_point_y;
      properties.anchor_point_y = *anchor_point_y;
    }

  if (const iscool::optional<std::string> color = style.get_string("color"))
    {
      properties.flags |= bim::axmol::style::display_property_flags::color;
      properties.color = ax::Color3B(m_colour_chart.to_color_4b(*color));
    }

  if (const iscool::optional<float> rotation = style.get_number("rotation"))
    {
      properties.flags |= bim::axmol::style::display_property_flags::rotation;
      properties.rotation = *rotation;
    }

  if (const iscool::optional<float> opacity = style.get_number("opacity"))
    {
      properties.flags |= bim::axmol::style::display_property_flags::opacity;
      properties.opacity = *opacity;
    }

  if (const iscool::optional<float> z_order = style.get_number("z-order"))
    {
      properties.flags |= bim::axmol::style::display_property_flags::z_order;
      properties.z_order = *z_order;
    }

  if (const iscool::optional<bool> cascade_opacity =
          style.get_boolean("cascade-opacity"))
    {
      properties.flags |=
          bim::axmol::style::display_property_flags::cascade_opacity;
      properties.cascade_opacity = *cascade_opacity;
    }

  if (const iscool::optional<bool> cascade_color =
          style.get_boolean("cascade-color"))
    {
      properties.flags |=
          bim::axmol::style::display_property_flags::cascade_color;
      properties.cascade_color = *cascade_color;
    }

  if (const iscool::optional<bool> visible = style.get_boolean("visible"))
    {
      properties.flags |= bim::axmol::style::display_property_flags::visible;
      properties.visible = *visible;
    }

  return properties;
}
