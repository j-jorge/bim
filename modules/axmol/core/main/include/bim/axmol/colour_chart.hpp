#pragma once

#include <iscool/strings/hash.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string_view>

namespace ax
{
  struct Color4B;
}

namespace bim::axmol
{
  class colour_chart
  {
  public:
    colour_chart();
    ~colour_chart();

    void add_alias(std::string name, std::string_view color);

    ax::Color4B to_color_4b(std::string_view color) const;

  private:
    using alias_map =
        boost::unordered_flat_map<std::string, ax::Color4B,
                                  iscool::strings::hash, std::equal_to<>>;

  private:
    alias_map m_aliases;
  };

}
