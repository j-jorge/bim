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

  void instantiate_widgets(named_node_group& nodes,
                           std::span<const std::string_view> excluded,
                           const bim::axmol::widget::context& context,
                           const iscool::style::declaration& style);
}
