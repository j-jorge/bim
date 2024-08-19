// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/add_group_as_children.hpp>

#include <axmol/2d/Node.h>

void bim::axmol::widget::add_group_as_children(ax::Node& parent,
                                               const named_node_group& group)
{
  for (const auto& [_, node_pointer] : group)
    {
      assert(node_pointer != nullptr);
      if (node_pointer->getParent() == nullptr)
        parent.addChild(node_pointer.get());
    }
}
