// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/style/apply_size.hpp>

#include <bim/axmol/style/bounds_properties.hpp>
#include <bim/axmol/style/bounds_property_flags.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/Node.h>
#include <axmol/2d/Sprite.h>
#include <axmol/2d/SpriteFrame.h>

static bool apply_width(float& width, const ax::Node& reference,
                        const bim::axmol::style::bounds_properties& bounds);
static bool apply_height(float& height, const ax::Node& reference,
                         const bim::axmol::style::bounds_properties& bounds);

void bim::axmol::style::apply_size(
    ax::Node& node, const ax::Node& reference,
    const bim::axmol::style::bounds_properties& bounds)
{
  ax::Vec2 size = node.getContentSize();

  if (bool(bounds.flags
           & bim::axmol::style::bounds_property_flags::width_ratio))
    {
      const ax::Vec2 reference_size = reference.getContentSize();

      apply_height(size.y, reference, bounds);
      size.x = size.y * bounds.ratio;

      if (bool(bounds.flags
               & bim::axmol::style::bounds_property_flags::max_percents_size)
          && (100 * size.x / reference_size.x > bounds.max_percents_size))
        {
          size.x =
              reference.getContentSize().x * bounds.max_percents_size / 100;
          size.y = size.x / bounds.ratio;
        }
    }
  else if (bool(bounds.flags
                & bim::axmol::style::bounds_property_flags::height_ratio))
    {
      apply_width(size.x, reference, bounds);
      size.y = size.x * bounds.ratio;

      const ax::Vec2 reference_size = reference.getContentSize();

      if (bool(bounds.flags
               & bim::axmol::style::bounds_property_flags::max_percents_size)
          && (100 * size.y / reference_size.y > bounds.max_percents_size))
        {
          size.y =
              reference.getContentSize().y * bounds.max_percents_size / 100;
          size.x = size.y / bounds.ratio;
        }
    }
  else
    {
      float ratio;
      bool keep_ratio = false;

      if (bounds.keep_ratio)
        {
          if (const ax::Sprite* const sprite =
                  dynamic_cast<ax::Sprite*>(&node))
            if (const ax::SpriteFrame* const sprite_frame =
                    sprite->getSpriteFrame())
              {
                const ax::Vec2& sprite_size = sprite_frame->getOriginalSize();

                ratio = sprite_size.x / sprite_size.y;
                keep_ratio = !std::isnan(ratio);
              }
        }

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
