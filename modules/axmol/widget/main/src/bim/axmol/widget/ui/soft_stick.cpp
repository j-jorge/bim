// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/soft_stick.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/input/observer/soft_stick.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <axmol/2d/Sprite.h>

#define x_widget_scope bim::axmol::widget::soft_stick::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Sprite, stick)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim_implement_widget(bim::axmol::widget::soft_stick);

bim::axmol::widget::soft_stick::soft_stick(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_soft_stick_input(*this)
  , m_style_bounds(style.get_declaration_or_empty("bounds"))
{
  m_soft_stick_input->connect_to_changed(
      [this]()
      {
        update_display();
      });

  m_inputs.attach_to_root(m_soft_stick_input);
}

bim::axmol::widget::soft_stick::~soft_stick() = default;

bim::axmol::input::node_reference
bim::axmol::widget::soft_stick::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::soft_stick::onEnter()
{
  ax::Node::onEnter();

  update_display();
}

void bim::axmol::widget::soft_stick::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  update_display();
}

void bim::axmol::widget::soft_stick::enable(bool enabled)
{
  if (m_soft_stick_input->is_enabled() == enabled)
    return;

  m_soft_stick_input->enable(enabled);
  update_display();
}

const ax::Vec2& bim::axmol::widget::soft_stick::drag() const
{
  return m_soft_stick_input->drag();
}

bool bim::axmol::widget::soft_stick::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}

void bim::axmol::widget::soft_stick::update_display()
{
  apply_bounds(m_context.style_cache, m_controls->all_nodes, m_style_bounds);

  const ax::Vec2 size = getContentSize() / 2;
  m_controls->stick->setPosition(size + m_soft_stick_input->drag() * size);
}
