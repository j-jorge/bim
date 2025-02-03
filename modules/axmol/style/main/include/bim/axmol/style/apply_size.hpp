// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace ax
{
  class Node;
}

namespace bim::axmol::style
{
  class bounds_properties;

  void apply_size(ax::Node& node, const ax::Node& reference,
                  const bounds_properties& bounds);

}
