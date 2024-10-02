// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/set_node_parents.hpp>

#include <bim/axmol/widget/log_context.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/Node.h>

void bim::axmol::widget::set_node_parents(
    const named_node_group& nodes, const iscool::style::declaration& style)
{
  const named_node_group::const_iterator nodes_end = nodes.end();

  for (const auto& [name, node] : nodes)
    {
      const iscool::optional<const std::string&> parent_name =
          style.get_declaration(name)->get_string("parent");

      if (!parent_name)
        continue;

      const named_node_group::const_iterator parent_it =
          nodes.find(*parent_name);

      if (parent_it == nodes_end)
        {
          ic_log(iscool::log::nature::error(),
                 bim::axmol::widget::g_log_context,
                 "No reference node named '%s' to set as parent node for %s.",
                 *parent_name, name);
          continue;
        }

      parent_it->second->addChild(node.get());
    }
}
