// SPDX-License-Identifier: AGPL-3.0-only
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
    class dynamic_factory;

    [[nodiscard]] bim::axmol::ref_ptr<ax::ActionInterval>
    repeat_from_style(const dynamic_factory& factory,
                      const bim::axmol::colour_chart& colors,
                      const iscool::style::declaration& style);
  }
}
