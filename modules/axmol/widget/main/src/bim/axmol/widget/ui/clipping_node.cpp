// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/clipping_node.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/factory/clipping_node.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/2d/ClippingNode.h>

#define x_widget_scope bim::axmol::widget::clipping_node::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::ClippingNode, clipping_node)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim_implement_widget(bim::axmol::widget::clipping_node);

bim::axmol::widget::clipping_node::clipping_node(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_stencil(*m_controls->all_nodes.find("stencil")->second)
{
  m_controls->clipping_node->setStencil(&m_stencil);
}

bim::axmol::widget::clipping_node::~clipping_node() = default;

void bim::axmol::widget::clipping_node::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  // Add the stencil just to update its bounds.
  addChild(&m_stencil);
  apply_bounds(m_context, m_controls->all_nodes, m_style_bounds);
  m_stencil.removeFromParent();
}

bool bim::axmol::widget::clipping_node::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);
  m_stencil.removeFromParent();

  return true;
}
