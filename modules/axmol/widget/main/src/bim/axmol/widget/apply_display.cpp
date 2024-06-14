#include <bim/axmol/widget/apply_display.hpp>

#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/unreachable.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/style/declaration.hpp>

void bim::axmol::widget::apply_display(bim::axmol::style::cache& style_cache,
                                       const named_node_group& nodes,
                                       const iscool::style::declaration& style)
{
  const bim::axmol::widget::named_node_group::const_iterator nodes_end =
      nodes.end();

  for (const auto& e : style.get_declarations())
    {
      const std::string& node_name = e.first;
      const iscool::style::declaration& substyle = e.second;

      const bim::axmol::widget::named_node_group::const_iterator node =
          nodes.find(node_name);

      if (node == nodes_end)
        {
          bim_unreachable_in_release;
          ic_log(iscool::log::nature::error(),
                 bim::axmol::widget::g_log_context, "No node named '{}'.",
                 node_name);
          continue;
        }

      bim::axmol::style::apply_display(style_cache.get_display(substyle),
                                       *node->second);
    }
}
