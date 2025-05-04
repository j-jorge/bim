// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <string_view>

namespace ax
{
  class Node;
}

namespace bim::axmol
{
  /**
   * Returns the child or subchild of the given node by following the
   * /-separated names from the given path.
   */
  ax::Node* find_child_by_path(ax::Node& node, std::string_view path);
}
