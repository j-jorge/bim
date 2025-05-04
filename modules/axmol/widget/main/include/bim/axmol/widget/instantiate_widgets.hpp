// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

#include <span>
#include <string_view>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;

  bim::axmol::ref_ptr<ax::Node>
  instantiate_widget(const bim::axmol::widget::context& context,
                     const iscool::style::declaration& style);

  void instantiate_widgets(named_node_group& nodes,
                           std::span<const std::string_view> excluded,
                           const bim::axmol::widget::context& context,
                           const iscool::style::declaration& style);
}
