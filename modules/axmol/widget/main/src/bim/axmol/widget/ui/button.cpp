// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#define x_widget_scope bim::axmol::widget::button::
#define x_widget_type_name controls
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/Action.h>
#include <axmol/2d/ActionInterval.h>

bim_implement_widget(bim::axmol::widget::button);

bim::axmol::widget::button::button(const bim::axmol::widget::context& context,
                                   const iscool::style::declaration& style)
  : m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_container(ax::Node::create())
  , m_behavior(context, style, *m_container, m_controls->all_nodes)
{
  m_container->setName("container");
  m_container->setAnchorPoint(ax::Vec2(0.5, 0.5));
  m_container->setCascadeColorEnabled(true);
  m_container->setCascadeOpacityEnabled(true);
}

bim::axmol::widget::button::~button() = default;

iscool::signals::connection
bim::axmol::widget::button::connect_to_clicked(std::function<void()> f)
{
  return m_behavior.connect_to_clicked(std::move(f));
}

iscool::signals::connection
bim::axmol::widget::button::connect_to_pressed(std::function<void()> f)
{
  return m_behavior.connect_to_pressed(std::move(f));
}

bim::axmol::input::node_reference
bim::axmol::widget::button::input_node() const
{
  return m_behavior.input_node();
}

void bim::axmol::widget::button::onEnter()
{
  ax::Node::onEnter();
  m_behavior.on_enter();
}

void bim::axmol::widget::button::onExit()
{
  m_behavior.on_exit();
  ax::Node::onExit();
}

void bim::axmol::widget::button::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  m_container->setContentSize(size);
  m_container->setPosition(size / 2);

  m_behavior.container_size_changed();
}

void bim::axmol::widget::button::enable(bool enabled)
{
  m_behavior.enable(enabled);
}

void bim::axmol::widget::button::cancel_on_swipe(bool v)
{
  m_behavior.cancel_on_swipe(v);
}

bool bim::axmol::widget::button::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  addChild(m_container.get());

  add_group_as_children(*m_container, m_controls->all_nodes);

  return true;
}
