// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/instantiate_widgets.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/dynamic_factory.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/Node.h>

#include <algorithm>

bim::axmol::ref_ptr<ax::Node> bim::axmol::widget::instantiate_widget(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  const iscool::optional<const std::string&> type =
      style.get_string("instantiate");

  if (!type)
    return {};

  return context.factory.create(*type, context, style);
}

void bim::axmol::widget::instantiate_widgets(
    named_node_group& nodes, std::span<const std::string_view> excluded,
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{

  for (const iscool::style::declaration::declaration_map::value_type& d :
       style.get_declarations())
    {
      if (std::find(excluded.begin(), excluded.end(), d.first)
          != excluded.end())
        continue;

      const iscool::optional<const std::string&> type =
          d.second->get_string("instantiate");

      if (!type)
        continue;

      ref_ptr<ax::Node> node =
          context.factory.create(*type, context, *d.second);
      node->setName(d.first);
      nodes.emplace(d.first, std::move(node));
    }
}
