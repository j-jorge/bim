#pragma once

#include <iscool/context.hpp>

namespace bim::axmol
{
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
      bim::axmol::style::cache& style_cache;
      bim::axmol::widget::dynamic_factory& factory;
    };
  }
}
