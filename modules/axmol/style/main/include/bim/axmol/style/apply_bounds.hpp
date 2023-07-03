#pragma once

namespace ax
{
  class Node;
}

namespace bim::axmol::style
{
  class bounds_properties;

  void apply_bounds(const bounds_properties& bounds, ax::Node& node,
                    const ax::Node* reference);

}
