#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

namespace bim::axmol::widget
{
  void add_group_as_children(ax::Node& parent, const named_node_group& group);
}
