#pragma once

#include <iscool/context.hpp>

namespace bim::axmol
{
  class colour_chart;

  namespace style
  {
    class cache;
  }

  namespace widget
  {
    class dynamic_factory;

    class context
    {
    public:
      const bim::axmol::colour_chart& colors;
      bim::axmol::style::cache& style_cache;
      const bim::axmol::widget::dynamic_factory& factory;
    };
  }
}
