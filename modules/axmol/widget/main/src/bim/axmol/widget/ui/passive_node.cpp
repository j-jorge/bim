// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/passive_node.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#define x_widget_scope bim::axmol::widget::passive_node::
#define x_widget_type_name controls
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim_implement_widget(bim::axmol::widget::passive_node);

bim::axmol::widget::passive_node::passive_node(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_dirty(false)
{
  const iscool::optional<const iscool::style::declaration&> bounds =
      style.get_declaration("bounds");

  if (bounds)
    {
      add_group_as_children(*this, m_controls->all_nodes);
      m_style_bounds = &*bounds;
      m_dirty = true;
    }
  else
    m_style_bounds = nullptr;
}

bim::axmol::widget::passive_node::~passive_node() = default;

void bim::axmol::widget::passive_node::onEnter()
{
  ax::Node::onEnter();

  if (m_dirty)
    update_bounds();
}

void bim::axmol::widget::passive_node::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  update_bounds();
}

void bim::axmol::widget::passive_node::fill(
    const named_node_group& nodes, const iscool::style::declaration& bounds)
{
  removeAllChildren();

  m_nodes = nodes;
  add_group_as_children(*this, m_nodes);

  m_style_bounds = &bounds;

  update_bounds();
}

void bim::axmol::widget::passive_node::update_bounds()
{
  if (!isRunning())
    {
      m_dirty = true;
      return;
    }

  if (m_style_bounds)
    {
      if (!m_nodes.empty())
        apply_bounds(m_context, m_nodes, *m_style_bounds);
      else
        apply_bounds(m_context, m_controls->all_nodes, *m_style_bounds);
    }

  m_dirty = false;
}
