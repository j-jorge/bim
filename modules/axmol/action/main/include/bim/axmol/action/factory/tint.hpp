#pragma once

#include <bim/axmol/ref_ptr.hpp>

namespace iscool::style
{
  class declaration;
}

namespace ax
{
  class ActionInterval;
}

namespace bim::axmol
{
  class colour_chart;

  namespace action
  {
    [[nodiscard]] bim::axmol::ref_ptr<ax::ActionInterval>
    tint_from_style(const bim::axmol::colour_chart& colors,
                    const iscool::style::declaration& style);
  }
}
