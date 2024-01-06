#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

namespace bim::axmol::style
{
  class cache;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  void apply_display(bim::axmol::style::cache& style_cache,
                     const named_node_group& nodes,
                     const iscool::style::declaration& style);
}
