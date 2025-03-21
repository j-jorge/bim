// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/apply_bounds.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_bounds.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/unreachable.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <axmol/2d/Node.h>

#include <cassert>

namespace
{
  struct styling
  {
    ax::Node* reference;
    const iscool::style::declaration* style;
    bool done = false;
  };

  using styling_queue = boost::unordered_flat_map<ax::Node*, styling>;
}

static void apply_styling(const bim::axmol::widget::context& context,
                          styling_queue& queue, ax::Node& node, styling& s)
{
  if (s.done)
    return;

  s.done = true;

  if (s.reference)
    {
      styling_queue::iterator r = queue.find(s.reference);

      // If the reference is in the queue then it must be processed before the
      // current node. Otherwise the reference may either be the parent node or
      // it may have been set in another call. In both cases we consider the
      // reference to be ready and proceed with the current node.
      if (r != queue.end())
        apply_styling(context, queue, *r->first, r->second);
    }

  bim::axmol::style::apply_bounds(context.style_cache.get_bounds(*s.style),
                                  node, *s.reference, context.device_scale);
}

static styling_queue
build_styling_queue(const bim::axmol::widget::named_node_group& nodes,
                    const iscool::style::declaration& style)
{
  styling_queue queue;
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

      iscool::optional<const std::string&> reference_name =
          substyle.get_string("reference");

      ax::Node* reference = nullptr;

      if (reference_name)
        {
          const bim::axmol::widget::named_node_group::const_iterator r =
              nodes.find(*reference_name);

          if (r == nodes_end)
            {
              bim_unreachable_in_release;
              ic_log(iscool::log::nature::error(),
                     bim::axmol::widget::g_log_context,
                     "No reference node named '{}'.", *reference_name);
              continue;
            }

          reference = r->second.get();
        }
      else
        reference = node->second->getParent();

      queue[node->second.get()] =
          styling{ .reference = reference, .style = &substyle };
    }

  return queue;
}

void bim::axmol::widget::apply_bounds(
    const bim::axmol::widget::context& context, const named_node_group& nodes,
    const iscool::style::declaration& style)
{
  styling_queue queue = build_styling_queue(nodes, style);

  for (auto& [node, styling] : queue)
    apply_styling(context, queue, *node, styling);
}
