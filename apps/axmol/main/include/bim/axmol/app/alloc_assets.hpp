// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <vector>

namespace bim::axmol::widget
{
  class context;
}

namespace ax
{
  class Node;
  class Sprite;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  void alloc_assets(std::vector<ax::Sprite*>& out,
                    const bim::axmol::widget::context& context,
                    std::size_t count, const iscool::style::declaration& style,
                    ax::Node& parent);
}
