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
  ttf_config.fontSize = style.get_number("font.size", 12);
  ttf_config.outlineSize = outline_size;
  ttf_config.italics = style.get_boolean("font.italics", false);
  ttf_config.bold = style.get_boolean("font.bold", false);
  ttf_config.underline = style.get_boolean("font.underline", false);
  ttf_config.strikethrough = style.get_boolean("font.strikethrough", false);

  const iscool::optional<const std::string&> horizontal_align_string =
      style.get_string("horizontal_align");
  ax::TextHAlignment horizontal_align = ax::TextHAlignment::LEFT;

  if (horizontal_align_string)
    {
      if (*horizontal_align_string == "center")
        horizontal_align = ax::TextHAlignment::CENTER;
      else if (*horizontal_align_string == "right")
        horizontal_align = ax::TextHAlignment::RIGHT;
      else if (*horizontal_align_string != "left")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Unknown text alignment: '%s'.", *horizontal_align_string);
    }

  iscool::optional<const std::string&> localized_text =
      style.get_string("text.i18n");
  const std::string text = localized_text ? ic_gettext(localized_text->c_str())
                                          : style.get_string("text", "");

  bim::axmol::ref_ptr<ax::Label> result =
      ax::Label::createWithTTF(ttf_config, text, horizontal_align);

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

      result->enableShadow(shadow_color, shadow_offset);
    }

  if (outline_size != 0)
    {
      const iscool::optional<const std::string&> outline_color_string =
          style.get_string("outline.color");

      const ax::Color4B outline_color =
          outline_color_string
              ? context.colors.to_color_4b(*outline_color_string)
              : ax::Color4B::BLACK;

      result->enableOutline(outline_color, outline_size);
    }

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
