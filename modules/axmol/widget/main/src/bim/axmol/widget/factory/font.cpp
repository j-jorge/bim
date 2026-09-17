ax::TTFConfig create_font(const bim::axmol::widget::context& context,
                          const iscool::style::declaration& style)
{
  ax::TTFConfig ttf_config;

  const iscool::optional<const std::string&> font_path =
      style.get_string("font.file");

  if (!font_path)
    {
      ic_log(iscool::log::nature::error(), g_log_context, "Missing font.");
      return ttf_config;
    }

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

  const int outline_size = style.get_number("outline.size", 0);
  ttf_config.outlineSize = outline_size;

  ttf_config.fontSize =
      style.get_number("font.size", 12) * context.device_scale;
  ttf_config.bold = style.get_boolean("font.bold", false);
  ttf_config.underline = style.get_boolean("font.underline", false);
  ttf_config.strikethrough = style.get_boolean("font.strikethrough", false);

  return ttf_config;
}
