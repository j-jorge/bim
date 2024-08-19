// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace ax
{
  class Node;
  class Rect;
}

namespace bim::axmol
{
  /// The bounding box of a node in screen's coordinates.
  ax::Rect bounding_box_on_screen(const ax::Node& node);
}
