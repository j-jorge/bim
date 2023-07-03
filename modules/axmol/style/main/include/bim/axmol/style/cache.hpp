#pragma once

#include <boost/unordered/unordered_flat_map.hpp>

#include <cstdint>

namespace iscool::style
{
  class declaration;

}
namespace bim::axmol
{
  class colour_chart;

  namespace style
  {
    class bounds_properties;
    class display_properties;

    class cache
    {
    public:
      explicit cache(const bim::axmol::colour_chart& colour_chart);
      ~cache();

      const bounds_properties&
      get_bounds(const iscool::style::declaration& style);

      const display_properties&
      get_display(const iscool::style::declaration& style);

    private:
      using bounds_map =
          boost::unordered_flat_map<std::uint64_t, bounds_properties>;
      using display_map =
          boost::unordered_flat_map<std::uint64_t, display_properties>;

    private:
      const bim::axmol::colour_chart& m_colour_chart;
      bounds_map m_bounds;
      display_map m_display;
    };
  }
}
