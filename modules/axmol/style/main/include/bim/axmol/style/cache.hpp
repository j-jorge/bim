// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <boost/unordered/unordered_flat_map.hpp>

#include <cstdint>
#include <unordered_map>

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
      // We need reference stability for the bounds because the map may grow
      // while being read. For example, one has a reference to a
      // bounds_properties and uses it to resize a node. The resizing triggers
      // another computation of bounds for the child nodes, which will then
      // look-up more properties in the same map. Since the properties are
      // added lazily then it may cause a rehash or a reallocation, depending
      // on the internals of the map. Without reference stability the first
      // reference above would become invalid.
      using bounds_map = std::unordered_map<std::uint64_t, bounds_properties>;
      using display_map =
          boost::unordered_flat_map<std::uint64_t, display_properties>;

    private:
      const bim::axmol::colour_chart& m_colour_chart;
      bounds_map m_bounds;
      display_map m_display;
    };
  }
}
