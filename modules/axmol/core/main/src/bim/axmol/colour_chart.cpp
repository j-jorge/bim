#include <bim/axmol/colour_chart.hpp>

#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/error.hpp>

#include <axmol/base/Types.h>

#include <charconv>

static ax::Color3B parse_color(std::string_view color)
{
  if ((color.size() != 7) || (color[0] != '#'))
    {
      ic_causeless_log(iscool::log::nature::error(), "colour_chart",
                       "Unknown colour '%s'. Supported format is '#rrggbb'.\n",
                       color);
      return ax::Color3B::MAGENTA;
    }

  ax::Color3B result;
  constexpr int base = 16;

  if ((std::from_chars(color.data() + 1, color.data() + 3, result.r, base).ec
       == std::errc{})
      && (std::from_chars(color.data() + 3, color.data() + 5, result.g, base)
              .ec
          == std::errc{})
      && (std::from_chars(color.data() + 5, color.data() + 7, result.b, base)
              .ec
          == std::errc{}))
    return result;

  ic_causeless_log(
      iscool::log::nature::error(), "colour_chart",
      "Failed to parse color '%s'. Supported format is '#rrggbb'.\n", color);

  return ax::Color3B::MAGENTA;
}

bim::axmol::colour_chart::colour_chart() = default;
bim::axmol::colour_chart::~colour_chart() = default;

void bim::axmol::colour_chart::add_alias(std::string name,
                                         std::string_view color)
{
  m_aliases[std::move(name)] = parse_color(color);
}

ax::Color3B bim::axmol::colour_chart::to_color_3b(std::string_view color) const
{
  const alias_map::const_iterator it = m_aliases.find(color);

  if (it != m_aliases.end())
    return it->second;

  return parse_color(color);
}
