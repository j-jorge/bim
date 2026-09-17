// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/label_style.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/font_catalog.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/colour_chart.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/style/declaration.hpp>

bim::axmol::widget::label_style::label_style(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  const iscool::optional<const std::string&> font_path =
      style.get_string("font.file");

  if (!font_path)
    ic_log(iscool::log::nature::error(), g_log_context, "Missing font.");
  else
    {
      if (style.get_boolean("font.substitutable", true))
        {
          const bim::axmol::widget::font_catalog::resolve_result font =
              context.fonts.resolve(*font_path);
          ttf_config.fontFilePath = font.name;
          ttf_config.italics =
              font.force_italics || style.get_boolean("font.italics", false);
        }
      else
        {
          ttf_config.fontFilePath = *font_path;
          ttf_config.italics = style.get_boolean("font.italics", false);
        }
    }

  const int outline_size = style.get_number("outline.size", 0);
  ttf_config.outlineSize = outline_size;

  ttf_config.fontSize =
      style.get_number("font.size", 12) * context.device_scale;
  ttf_config.bold = style.get_boolean("font.bold", false);
  ttf_config.underline = style.get_boolean("font.underline", false);
  ttf_config.strikethrough = style.get_boolean("font.strikethrough", false);

  const iscool::optional<const std::string&> style_color =
      style.get_string("font.color");

  color = style_color ? context.colors.to_color_4b(*style_color)
                      : ax::Color4B::WHITE;

  const iscool::optional<const std::string&> horizontal_align_string =
      style.get_string("align.horizontal");
  horizontal_align = ax::TextHAlignment::LEFT;

  if (horizontal_align_string)
    {
      if (*horizontal_align_string == "center")
        horizontal_align = ax::TextHAlignment::CENTER;
      else if (*horizontal_align_string == "right")
        horizontal_align = ax::TextHAlignment::RIGHT;
      else if (*horizontal_align_string != "left")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown horizontal text alignment: '{}'.",
               *horizontal_align_string);
    }

  const iscool::optional<const std::string&> vertical_align_string =
      style.get_string("align.vertical");
  vertical_align = ax::TextVAlignment::CENTER;

  if (vertical_align_string)
    {
      if (*vertical_align_string == "top")
        vertical_align = ax::TextVAlignment::TOP;
      else if (*vertical_align_string == "bottom")
        vertical_align = ax::TextVAlignment::BOTTOM;
      else if (*vertical_align_string != "center")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown vertical text alignment: '{}'.",
               *vertical_align_string);
    }

  const iscool::optional<const std::string&> localized_text =
      style.get_string("text.i18n");
  text = localized_text ? ic_gettext(localized_text->c_str())
                        : style.get_string("text", "");
}
