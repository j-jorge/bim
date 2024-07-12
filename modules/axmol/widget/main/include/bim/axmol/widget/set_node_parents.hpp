#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;

  void set_node_parents(const named_node_group& nodes,
                        const iscool::style::declaration& style);
}
