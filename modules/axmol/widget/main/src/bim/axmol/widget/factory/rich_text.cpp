// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/rich_text.hpp>

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

#include <axmol/ui/UIRichText.h>

bim::axmol::ref_ptr<ax::ui::RichText>
bim::axmol::widget::factory<ax::ui::RichText>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::ValueMap defaults;

  const iscool::optional<const std::string&> font_path =
      style.get_string("font.file");

  if (font_path)
    defaults[ax::ui::RichText::KEY_FONT_FACE] = *font_path;
  else
    ic_log(iscool::log::nature::error(), g_log_context, "Missing font.");

  defaults[ax::ui::RichText::KEY_FONT_SIZE] =
      style.get_number("font.size", 12) * context.device_scale;
  defaults[ax::ui::RichText::KEY_TEXT_ITALIC] =
      style.get_boolean("font.italics", false);
  defaults[ax::ui::RichText::KEY_TEXT_BOLD] =
      style.get_boolean("font.bold", false);

  const iscool::optional<const std::string&> text_color_string =
      style.get_string("font.color");

  const ax::Color4B text_color =
      text_color_string ? context.colors.to_color_4b(*text_color_string)
                        : ax::Color4B::WHITE;

  defaults[ax::ui::RichText::KEY_FONT_COLOR_STRING] = fmt::format(
      "#{:02x}{:02x}{:02x}", text_color.r, text_color.g, text_color.b);

  const iscool::optional<const std::string&> link_color_string =
      style.get_string("link.color");

  const ax::Color4B link_color =
      link_color_string ? context.colors.to_color_4b(*link_color_string)
                        : ax::Color4B::WHITE;

  defaults[ax::ui::RichText::KEY_ANCHOR_FONT_COLOR_STRING] = fmt::format(
      "#{:02x}{:02x}{:02x}", link_color.r, link_color.g, link_color.b);
  defaults[ax::ui::RichText::KEY_ANCHOR_TEXT_LINE] =
      ax::ui::RichText::VALUE_TEXT_LINE_UNDER;

  const iscool::optional<const std::string&> horizontal_align_string =
      style.get_string("align.horizontal");
  ax::ui::RichText::HorizontalAlignment horizontal_align =
      ax::ui::RichText::HorizontalAlignment::LEFT;

  if (horizontal_align_string)
    {
      if (*horizontal_align_string == "center")
        horizontal_align = ax::ui::RichText::HorizontalAlignment::CENTER;
      else if (*horizontal_align_string == "right")
        horizontal_align = ax::ui::RichText::HorizontalAlignment::RIGHT;
      else if (*horizontal_align_string != "left")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown horizontal text alignment: '{}'.",
               *horizontal_align_string);
    }

  defaults[ax::ui::RichText::KEY_HORIZONTAL_ALIGNMENT] = (int)horizontal_align;

  const iscool::optional<const std::string&> vertical_align_string =
      style.get_string("align.vertical");
  ax::ui::RichText::VerticalAlignment vertical_align =
      ax::ui::RichText::VerticalAlignment::TOP;

  if (vertical_align_string)
    {
      if (*vertical_align_string == "center")
        vertical_align = ax::ui::RichText::VerticalAlignment::CENTER;
      else if (*vertical_align_string == "bottom")
        vertical_align = ax::ui::RichText::VerticalAlignment::BOTTOM;
      else if (*vertical_align_string != "top")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown vertical text alignment: '{}'.",
               *vertical_align_string);
    }

  defaults[ax::ui::RichText::KEY_VERTICAL_ALIGNMENT] = (int)vertical_align;

  iscool::optional<const std::string&> localized_text =
      style.get_string("text.i18n");
  const std::string text = localized_text ? ic_gettext(localized_text->c_str())
                                          : style.get_string("text", "");

  bim::axmol::ref_ptr<ax::ui::RichText> result =
      ax::ui::RichText::createWithXML(text, defaults);

  result->ignoreContentAdaptWithSize(false);

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
