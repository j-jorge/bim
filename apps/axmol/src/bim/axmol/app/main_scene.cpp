// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/main_scene.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory/node.hpp>

#include <bim/assume.hpp>

#define x_widget_scope bim::axmol::app::main_scene::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Node, overlay) x_widget(ax::Node, main)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/Scene.h>

bim::axmol::app::main_scene::main_scene(
    ax::Scene& scene, const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_scene(scene)
  , m_controls(context, *style.get_declaration("widgets"))
{
  m_inputs.push_back(m_overlay_inputs.root());
  m_inputs.push_back(m_main_canvas_inputs.root());

  bim::axmol::widget::add_group_as_children(scene, m_controls->all_nodes);
  bim::axmol::widget::apply_bounds(context.style_cache, m_controls->all_nodes,
                                   *style.get_declaration("bounds"));
}

bim::axmol::app::main_scene::~main_scene()
{
  m_scene.removeAllChildren();
}

bim::axmol::input::node_reference
bim::axmol::app::main_scene::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::app::main_scene::add_in_main_canvas(
    ax::Node& node, const bim::axmol::input::node_reference& inputs)
{
  m_controls->main->addChild(&node);

  const ax::Vec2 size = m_controls->main->getContentSize();
  node.setContentSize(size);

  // Align the node's bottom-left corner with its parent's.
  node.setPosition(node.getAnchorPoint() * size);

  m_main_canvas_inputs.push_back(inputs);
}

void bim::axmol::app::main_scene::add_in_overlays(
    ax::Node& node, const bim::axmol::input::node_reference& inputs)
{
  m_controls->overlay->addChild(&node);

  m_overlay_inputs.push_back(inputs);
  m_overlay_node_inputs.insert(node_to_inputs_map::value_type(&node, inputs));
}

void bim::axmol::app::main_scene::remove_from_overlays(ax::Node& node)
{
  const node_to_inputs_map::iterator it = m_overlay_node_inputs.find(&node);
  bim_assume(it != m_overlay_node_inputs.end());

  m_overlay_inputs.erase(it->second);
  m_overlay_node_inputs.erase(it);

  m_controls->overlay->removeChild(&node);
}
