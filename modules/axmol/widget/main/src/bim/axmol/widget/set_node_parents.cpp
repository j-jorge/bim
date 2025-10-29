// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/set_node_parents.hpp>

#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/find_child_by_path.hpp>

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

      const std::string_view node_path = *parent_name;

      const std::string_view::size_type slash = node_path.find_first_of('/');
      const bool has_child_path = slash != std::string_view::npos;

      const std::string_view root_name =
          has_child_path ? node_path.substr(0, slash) : node_path;

      const named_node_group::const_iterator parent_it = nodes.find(root_name);

      if (parent_it != nodes_end)
        {
          ax::Node* const parent =
              has_child_path ? find_child_by_path(*parent_it->second,
                                                  node_path.substr(slash + 1))
                             : parent_it->second.get();

          if (parent)
            {
              parent->addChild(node.get());
              continue;
            }
        }

      ic_log(iscool::log::nature::error(), bim::axmol::widget::g_log_context,
             "No reference node named '{}' to set as parent node for {}.",
             root_name, name);
    }
}
