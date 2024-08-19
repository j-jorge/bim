// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace ax
{
  class Node;
}

namespace bim::axmol::style
{
  class display_properties;

  void apply_display(const display_properties& display, ax::Node& node);

}
