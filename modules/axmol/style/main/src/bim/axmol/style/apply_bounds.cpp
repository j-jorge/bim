// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/style/apply_bounds.hpp>

#include <bim/axmol/style/apply_size.hpp>

#include <bim/axmol/style/bounds_properties.hpp>
#include <bim/axmol/style/bounds_property_flags.hpp>
#include <bim/axmol/style/scale_mode.hpp>

#include <axmol/2d/Node.h>

#include <cassert>

static void apply_scale(ax::Node& node, const ax::Node& reference,
                        const bim::axmol::style::bounds_properties& bounds,
                        float device_scale);
static void apply_position(ax::Node& node, const ax::Node& reference,
                           const bim::axmol::style::bounds_properties& bounds,
                           float device_scale);
static void set_bottom_left(ax::Node& node,
                            const bim::axmol::style::bounds_properties& bounds,
                            const ax::Vec2& reference_bottom_left,
                            const ax::Vec2& reference_size,
                            float device_scale);

void bim::axmol::style::apply_bounds(const bounds_properties& bounds,
                                     ax::Node& node, const ax::Node& reference,
                                     float device_scale)
{
  assert((&reference == node.getParent())
         || (reference.getParent() == node.getParent()));

  apply_size(node, reference, bounds);
  apply_scale(node, reference, bounds, device_scale);
  apply_position(node, reference, bounds, device_scale);
}

void apply_scale(ax::Node& node, const ax::Node& reference,
                 const bim::axmol::style::bounds_properties& bounds,
                 float device_scale)
{
  if (bool(bounds.flags & bim::axmol::style::bounds_property_flags::scale))
    {
      node.setScale(bounds.scale);
      return;
    }

  const ax::Vec2& size = node.getContentSize();
  const ax::Vec2 reference_size =
      (&reference == node.getParent())
          ? reference.getContentSize()
          : (reference.getContentSize() * reference.getScale());
  const ax::Vec2 scale_to_reference = reference_size / size;

  if (bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::scale_mode))
    {
      switch (bounds.scale_mode)
        {
        case bim::axmol::style::scale_mode::cover:
          node.setScale(std::min(scale_to_reference.x, scale_to_reference.y));
          break;
        case bim::axmol::style::scale_mode::device:
          node.setScale(device_scale);
          break;
        case bim::axmol::style::scale_mode::fit:
          node.setScale(std::max(scale_to_reference.x, scale_to_reference.y));
          break;
        }

      return;
    }

  const bool has_max_percents_width =
      bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::
               scale_constraint_max_percents_width);
  const bool has_max_percents_height =
      bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::
               scale_constraint_max_percents_height);

  if (!has_max_percents_width && !has_max_percents_height)
    return;

  ax::Vec2 target_size;

  if (has_max_percents_width)
    target_size.x =
        reference_size.x * bounds.scale_constraint_max_percents_width / 100;
  else
    target_size.x = std::numeric_limits<float>::infinity();

  if (has_max_percents_height)
    target_size.y =
        reference_size.y * bounds.scale_constraint_max_percents_height / 100;
  else
    target_size.y = std::numeric_limits<float>::infinity();

  const ax::Vec2 scale = target_size / size;
  const float min_scale = std::min(scale.x, scale.y);

  if (std::isnan(min_scale))
    return;

  node.setScale(min_scale);
}

void apply_position(ax::Node& node, const ax::Node& reference,
                    const bim::axmol::style::bounds_properties& bounds,
                    float device_scale)
{
  if (&reference == node.getParent())
    set_bottom_left(node, bounds, ax::Vec2::ZERO, reference.getContentSize(),
                    device_scale);
  else
    set_bottom_left(
        node, bounds,
        reference.getPosition()
            - reference.getContentSize() * reference.getAnchorPoint(),
        reference.getContentSize() * reference.getScale(), device_scale);
}

void set_bottom_left(ax::Node& node,
                     const bim::axmol::style::bounds_properties& bounds,
                     const ax::Vec2& reference_bottom_left,
                     const ax::Vec2& reference_size, float device_scale)
{
  const ax::Vec2 node_anchor(
      (bool(bounds.flags & bim::axmol::style::bounds_property_flags::anchor_x)
           ? bounds.anchor_x
           : 0.5f),
      (bool(bounds.flags & bim::axmol::style::bounds_property_flags::anchor_y)
           ? bounds.anchor_y
           : 0.5f));
  const ax::Vec2 reference_anchor(
      (bool(bounds.flags
            & bim::axmol::style::bounds_property_flags::anchor_in_reference_x)
           ? bounds.anchor_in_reference_x
           : 0.5f),
      (bool(bounds.flags
            & bim::axmol::style::bounds_property_flags::anchor_in_reference_y)
           ? bounds.anchor_in_reference_y
           : 0.5f));
  const ax::Vec2 node_offset(
      (bool(bounds.flags & bim::axmol::style::bounds_property_flags::offset_x)
           ? (bounds.offset_x * device_scale)
           : 0.f),
      (bool(bounds.flags & bim::axmol::style::bounds_property_flags::offset_y)
           ? (bounds.offset_y * device_scale)
           : 0.f));

  const ax::Vec2 size = node.getContentSize();
  const ax::Vec2 scaled_size = size * node.getScale();

  node.setPosition(
      node_offset + reference_bottom_left + reference_anchor * reference_size
      - node_anchor * scaled_size + node.getAnchorPoint() * scaled_size);
}
