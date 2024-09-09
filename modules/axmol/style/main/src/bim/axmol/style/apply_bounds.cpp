// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/style/apply_bounds.hpp>

#include <bim/axmol/style/bounds_properties.hpp>
#include <bim/axmol/style/bounds_property_flags.hpp>
#include <bim/axmol/style/scale_mode.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/Node.h>

#include <cassert>

static void apply_size(ax::Node& node, const ax::Node& reference,
                       const bim::axmol::style::bounds_properties& bounds);
static bool apply_width(float& width, const ax::Node& reference,
                        const bim::axmol::style::bounds_properties& bounds);
static bool apply_height(float& height, const ax::Node& reference,
                         const bim::axmol::style::bounds_properties& bounds);
static void apply_scale(ax::Node& node, const ax::Node& reference,
                        const bim::axmol::style::bounds_properties& bounds);
static void apply_position(ax::Node& node, const ax::Node& reference,
                           const bim::axmol::style::bounds_properties& bounds);
static void set_bottom_left(ax::Node& node,
                            const bim::axmol::style::bounds_properties& bounds,
                            const ax::Vec2& reference_bottom_left,
                            const ax::Vec2& reference_size);

void bim::axmol::style::apply_bounds(const bounds_properties& bounds,
                                     ax::Node& node, const ax::Node& reference)
{
  assert((&reference == node.getParent())
         || (reference.getParent() == node.getParent()));

  apply_size(node, reference, bounds);
  apply_scale(node, reference, bounds);
  apply_position(node, reference, bounds);
}

void apply_size(ax::Node& node, const ax::Node& reference,
                const bim::axmol::style::bounds_properties& bounds)
{
  ax::Vec2 size = node.getContentSize();

  if (bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::width_ratio))
    {
      apply_height(size.y, reference, bounds);
      size.x = size.y * bounds.ratio;
    }
  else if (bool(bounds.flags
                & bim::axmol::style::bounds_property_flags::height_ratio))
    {
      apply_width(size.x, reference, bounds);
      size.y = size.x * bounds.ratio;
    }
  else
    {
      const float ratio = size.x / size.y;
      const bool keep_ratio = bounds.keep_ratio && !std::isnan(ratio);

      if (apply_width(size.x, reference, bounds) && keep_ratio)
        size.y = size.x / ratio;

      if (apply_height(size.y, reference, bounds) && keep_ratio)
        size.x = ratio * size.y;
    }

  if (size.x <= 0)
    size.x = 0;

  if (size.y <= 0)
    size.y = 0;

  node.setContentSize(size);

  ax::Label* const label = dynamic_cast<ax::Label*>(&node);

  if (label)
    label->setMaxLineWidth(size.x);
}

bool apply_width(float& width, const ax::Node& reference,
                 const bim::axmol::style::bounds_properties& bounds)
{
  if (bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::percents_width))
    {
      width = reference.getContentSize().x * bounds.percents_width / 100;
      return true;
    }

  return false;
}

bool apply_height(float& height, const ax::Node& reference,
                  const bim::axmol::style::bounds_properties& bounds)
{
  if (bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::percents_height))
    {
      height = reference.getContentSize().y * bounds.percents_height / 100;
      return true;
    }

  return false;
}

void apply_scale(ax::Node& node, const ax::Node& reference,
                 const bim::axmol::style::bounds_properties& bounds)
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
                    const bim::axmol::style::bounds_properties& bounds)
{
  if (&reference == node.getParent())
    set_bottom_left(node, bounds, ax::Vec2::ZERO, reference.getContentSize());
  else
    set_bottom_left(node, bounds,
                    reference.getPosition()
                        - reference.getContentSize()
                              * reference.getAnchorPoint(),
                    reference.getContentSize() * reference.getScale());
}

void set_bottom_left(ax::Node& node,
                     const bim::axmol::style::bounds_properties& bounds,
                     const ax::Vec2& reference_bottom_left,
                     const ax::Vec2& reference_size)
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

  const ax::Vec2 size = node.getContentSize();
  const ax::Vec2 scaled_size = size * node.getScale();

  node.setPosition(reference_bottom_left + reference_anchor * reference_size
                   - node_anchor * scaled_size
                   + node.getAnchorPoint() * scaled_size);
}
