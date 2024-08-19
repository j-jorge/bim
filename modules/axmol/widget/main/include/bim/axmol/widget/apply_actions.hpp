// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::action
{
  class runner;
}

namespace bim::axmol::widget
{
  class context;

  void apply_actions(bim::axmol::action::runner& runner,
                     const context& context, const named_node_group& nodes,
                     const iscool::style::declaration& style);
}
