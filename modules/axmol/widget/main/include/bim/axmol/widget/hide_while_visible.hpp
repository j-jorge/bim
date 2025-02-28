// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <vector>

namespace bim::axmol::widget
{
  template <typename T>
  void hide_while_visible(const std::vector<T*>& nodes,
                          std::size_t start_index)
  {
    for (std::size_t i = start_index, n = nodes.size(); i != n; ++i)
      {
        if (!nodes[i]->isVisible())
          return;

        nodes[i]->setVisible(false);
      }
  }
}
