#include <bim/axmol/style/apply_bounds.hpp>

#include <bim/axmol/style/bounds_properties.hpp>
#include <bim/axmol/style/bounds_property_flags.hpp>

#include <axmol/2d/Node.h>

static void apply_size(ax::Vec2& size, const ax::Node* reference,
                       const bim::axmol::style::bounds_properties& bounds);
static void apply_width(float& width, const ax::Node* reference,
                        const bim::axmol::style::bounds_properties& bounds);
static void apply_height(float& height, const ax::Node* reference,
                         const bim::axmol::style::bounds_properties& bounds);
static void
apply_scale_constraints(ax::Vec2& size, const ax::Node& reference,
                        const bim::axmol::style::bounds_properties& bounds);

void bim::axmol::style::apply_bounds(const bounds_properties& bounds,
                                     ax::Node& node, const ax::Node* reference)
{
  ax::Vec2 size = node.getContentSize();

  apply_size(size, reference, bounds);

  if (reference)
    apply_scale_constraints(size, *reference, bounds);

  if (size.x <= 0)
    size.x = 0;

  if (size.y <= 0)
    size.y = 0;

  node.setContentSize(size);
}

void apply_size(ax::Vec2& size, const ax::Node* reference,
                const bim::axmol::style::bounds_properties& bounds)
{
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
      apply_width(size.y, reference, bounds);
      apply_height(size.y, reference, bounds);
    }
}

void apply_width(float& width, const ax::Node* reference,
                 const bim::axmol::style::bounds_properties& bounds)
{
  if (reference
      && bool(bounds.flags
              & bim::axmol::style::bounds_property_flags::percents_width))
    width = reference->getContentSize().x * bounds.percents_width / 100;
}

void apply_height(float& height, const ax::Node* reference,
                  const bim::axmol::style::bounds_properties& bounds)
{
  if (reference
      && bool(bounds.flags
              & bim::axmol::style::bounds_property_flags::percents_height))
    height = reference->getContentSize().y * bounds.percents_height / 100;
}

void apply_scale_constraints(
    ax::Vec2& size, const ax::Node& reference,
    const bim::axmol::style::bounds_properties& bounds)
{
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

  const float ratio = size.x / size.y;

  if (std::isnan(ratio))
    return;

  const ax::Vec2& reference_size = reference.getContentSize();
  const bool keep_ratio = bounds.constraint_keep_ratio;

  if (has_max_percents_width)
    {
      const float percents_width(100 * size.x / reference_size.x);

      if (percents_width > bounds.scale_constraint_max_percents_width)
        {
          size.x = percents_width * reference_size.x / 100;

          if (keep_ratio)
            size.y = size.x / ratio;
        }
    }

  if (has_max_percents_height)
    {
      const float percents_height(100 * size.y / reference_size.y);

      if (percents_height > bounds.scale_constraint_max_percents_height)
        {
          size.y = percents_height * reference_size.y / 100;

          if (keep_ratio)
            size.x = size.y * ratio;
        }
    }
}
