// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/apply_actions.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/action/dynamic_factory.hpp>
#include <bim/axmol/action/runner.hpp>

#include <bim/unreachable.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInstant.h>
#include <axmol/2d/ActionInterval.h>

#include <cassert>

static bim::axmol::ref_ptr<ax::Spawn>
create_actions(const bim::axmol::widget::context& context,
               const bim::axmol::widget::named_node_group& nodes,
               const iscool::style::declaration& style)
{
  ax::Vector<ax::FiniteTimeAction*> actions;

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

      const bim::axmol::ref_ptr<ax::FiniteTimeAction> action =
          context.action_factory.create(context.colors, substyle);

      if (!action)
        {
          bim_unreachable_in_release;
          ic_log(iscool::log::nature::error(),
                 bim::axmol::widget::g_log_context, "No action for node '{}'.",
                 node_name);
          continue;
        }

      actions.pushBack(
          ax::TargetedAction::create(node->second.get(), action.get()));
    }

  if (actions.empty())
    return {};

  return ax::Spawn::create(actions);
}

void bim::axmol::widget::apply_actions(bim::axmol::action::runner& runner,
                                       const context& context,
                                       const named_node_group& nodes,
                                       const iscool::style::declaration& style)
{
  bim::axmol::ref_ptr<ax::Spawn> action =
      create_actions(context, nodes, style);

  if (action)
    runner.run(*action);
}

void bim::axmol::widget::apply_actions(bim::axmol::action::runner& runner,
                                       const context& context,
                                       const named_node_group& nodes,
                                       const iscool::style::declaration& style,
                                       std::function<void()> on_done)
{
  assert(on_done);

  bim::axmol::ref_ptr<ax::Spawn> action =
      create_actions(context, nodes, style);
  bim::axmol::ref_ptr<ax::CallFunc> callback(ax::CallFunc::create(on_done));

  if (action)
    runner.run(*ax::Sequence::create(action.get(), callback.get(), nullptr));
  else
    runner.run(*callback);
}
