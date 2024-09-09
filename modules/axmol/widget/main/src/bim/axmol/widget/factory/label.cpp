// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/label.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/colour_chart.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/Label.h>

bim::axmol::ref_ptr<ax::Label> bim::axmol::widget::factory<ax::Label>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  const iscool::optional<const std::string&> font_path =
      style.get_string("font.file");

  if (!font_path)
    {
      ic_log(iscool::log::nature::error(), g_log_context, "Missing font.");
      return ax::Label::create();
    }

  const int outline_size = style.get_number("outline.size", 0);

  ax::TTFConfig ttf_config;

  ttf_config.fontFilePath = *font_path;
  ttf_config.fontSize =
      style.get_number("font.size", 12) * context.device_scale;
  ttf_config.outlineSize = outline_size;
  ttf_config.italics = style.get_boolean("font.italics", false);
  ttf_config.bold = style.get_boolean("font.bold", false);
  ttf_config.underline = style.get_boolean("font.underline", false);
  ttf_config.strikethrough = style.get_boolean("font.strikethrough", false);

  const iscool::optional<const std::string&> horizontal_align_string =
      style.get_string("align.horizontal");
  ax::TextHAlignment horizontal_align = ax::TextHAlignment::LEFT;

  if (horizontal_align_string)
    {
      if (*horizontal_align_string == "center")
        horizontal_align = ax::TextHAlignment::CENTER;
      else if (*horizontal_align_string == "right")
        horizontal_align = ax::TextHAlignment::RIGHT;
      else if (*horizontal_align_string != "left")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown horizontal text alignment: '%s'.",
               *horizontal_align_string);
    }

  const iscool::optional<const std::string&> vertical_align_string =
      style.get_string("align.vertical");
  ax::TextVAlignment vertical_align = ax::TextVAlignment::CENTER;

  if (vertical_align_string)
    {
      if (*vertical_align_string == "top")
        vertical_align = ax::TextVAlignment::TOP;
      else if (*vertical_align_string == "bottom")
        vertical_align = ax::TextVAlignment::BOTTOM;
      else if (*vertical_align_string != "center")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown vertical text alignment: '%s'.",
               *vertical_align_string);
    }

  iscool::optional<const std::string&> localized_text =
      style.get_string("text.i18n");
  const std::string text = localized_text ? ic_gettext(localized_text->c_str())
                                          : style.get_string("text", "");

  bim::axmol::ref_ptr<ax::Label> result =
      ax::Label::createWithTTF(ttf_config, text, horizontal_align);
  result->setVerticalAlignment(vertical_align);

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

  iscool::optional<const std::string&> color = style.get_string("font.color");
  result->setTextColor(color ? context.colors.to_color_4b(*color)
                             : ax::Color4B::WHITE);

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

  if (outline_size != 0)
    {
      const iscool::optional<const std::string&> outline_color_string =
          style.get_string("outline.color");

      const ax::Color4B outline_color =
          outline_color_string
              ? context.colors.to_color_4b(*outline_color_string)
              : ax::Color4B::BLACK;

      result->enableOutline(outline_color,
                            outline_size * context.device_scale);
    }

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
