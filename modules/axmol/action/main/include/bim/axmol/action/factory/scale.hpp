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

namespace bim::axmol::action
{
  [[nodiscard]] bim::axmol::ref_ptr<ax::ActionInterval>
  scale_from_style(const iscool::style::declaration& style);
}
