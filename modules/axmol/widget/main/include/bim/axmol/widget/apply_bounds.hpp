// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;

  void apply_bounds(const context& context, const named_node_group& nodes,
                    const iscool::style::declaration& style);
}
