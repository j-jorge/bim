// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/app/application_event_dispatcher.hpp>
#include <bim/axmol/app/main_scene.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_actions.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/node.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/key_sink.hpp>
#include <bim/axmol/input/observer/touch_sink.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <axmol/2d/Node.h>

#define x_widget_scope bim::axmol::app::popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Node, container) x_widget(ax::Node, client_container)

#include <bim/axmol/widget/implement_controls_struct.hpp>

bim::axmol::app::popup::popup(const context& context,
                              const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(*context.get_widget_context(),
               *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_style_display_show(*style.get_declaration("display.show"))
  , m_style_action_show(*style.get_declaration("action.show"))
  , m_shown(false)
{
  m_inputs.attach_to_root(m_key_sink);
  m_inputs.attach_to_root(m_touch_sink);
}

bim::axmol::app::popup::~popup() = default;

void bim::axmol::app::popup::show(
    const bim::axmol::widget::named_node_group& nodes,
    const iscool::style::declaration& bounds,
    const bim::axmol::input::node_reference& inputs)
{
  if (m_shown)
    return;

  m_shown = true;

  m_context.get_main_scene()->add_in_overlays(*m_controls->container,
                                              m_inputs.root());
  bim::axmol::widget::apply_bounds(*m_context.get_widget_context(),
                                   m_controls->all_nodes, m_style_bounds);

  bim::axmol::widget::add_group_as_children(*m_controls->client_container,
                                            nodes);
  bim::axmol::widget::apply_bounds(*m_context.get_widget_context(), nodes,
                                   bounds);

  m_client_nodes.clear();

  for (const bim::axmol::widget::named_node_group::value_type& e : nodes)
    m_client_nodes.push_back(e.second.get());

  bim::axmol::widget::apply_display(
      m_context.get_widget_context()->style_cache, m_controls->all_nodes,
      m_style_display_show);
  bim::axmol::widget::apply_actions(
      m_action_runner, *m_context.get_widget_context(), m_controls->all_nodes,
      m_style_action_show,
      [this, inputs]()
      {
        m_inputs.push_back(inputs);
        m_context.get_event_dispatcher()->dispatch("popup-shown");
      });
}

void bim::axmol::app::popup::hide()
{
  if (!m_shown)
    return;

  m_action_runner.stop();

  m_shown = false;

  for (ax::Node* n : m_client_nodes)
    n->removeFromParent();

  m_inputs.pop_back();

  m_context.get_main_scene()->remove_from_overlays(*m_controls->container);
  m_context.get_event_dispatcher()->dispatch("popup-hidden");
}
