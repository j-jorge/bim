// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/label.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/label_style.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <axmol/2d/Label.h>

bim::axmol::ref_ptr<ax::Label> bim::axmol::widget::factory<ax::Label>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  const label_style ls(context, style);

  if (ls.ttf_config.fontFilePath.empty())
    return ax::Label::create();

  bim::axmol::ref_ptr<ax::Label> result =
      ax::Label::createWithTTF(ls.ttf_config, ls.text, ls.horizontal_align);

  result->setTextColor(ls.color);
  result->setVerticalAlignment(ls.vertical_align);
  result->enableWrap(style.get_boolean("wrap", false));

  iscool::optional<const std::string&> overflow_string =
      style.get_string("overflow");

  if (overflow_string)
    {
      if (*overflow_string == "none")
        result->setOverflow(ax::Label::Overflow::NONE);
      else if (*overflow_string == "clamp")
        result->setOverflow(ax::Label::Overflow::CLAMP);
      else if (*overflow_string == "shrink")
        result->setOverflow(ax::Label::Overflow::SHRINK);
      else if (*overflow_string == "resize-height")
        result->setOverflow(ax::Label::Overflow::RESIZE_HEIGHT);
    }

  const ax::Vec2 shadow_offset(style.get_number("shadow.offset.x", 0),
                               style.get_number("shadow.offset.y", 0));

  if (shadow_offset != ax::Vec2::ZERO)
    {
      const iscool::optional<const std::string&> shadow_color_string =
          style.get_string("shadow.color");

      const ax::Color4B shadow_color =
          shadow_color_string
              ? context.colors.to_color_4b(*shadow_color_string)
              : ax::Color4B::BLACK;

      result->enableShadow(shadow_color, shadow_offset * context.device_scale);
    }

  if (ls.ttf_config.outlineSize != 0)
    {
      const iscool::optional<const std::string&> outline_color_string =
          style.get_string("outline.color");

      const ax::Color4B outline_color =
          outline_color_string
              ? context.colors.to_color_4b(*outline_color_string)
              : ax::Color4B::BLACK;

      result->enableOutline(outline_color,
                            ls.ttf_config.outlineSize * context.device_scale);
    }

  result->setLineSpacing(style.get_number("line-spacing", 0)
                         * context.device_scale);

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
