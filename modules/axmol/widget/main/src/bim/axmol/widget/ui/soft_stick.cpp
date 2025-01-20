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
  , m_soft_stick_input(*this, *m_controls->stick)
  , m_style_bounds_left(style.get_declaration_or_empty("bounds.left"))
  , m_style_bounds_right(style.get_declaration_or_empty("bounds.right"))
  , m_on_the_left(true)
{
  m_soft_stick_input->connect_to_move(
      [this](const ax::Vec2& world_position)
      {
        move_stick_to(world_position);
      });
  m_soft_stick_input->connect_to_up(
      [this]()
      {
        move_stick_to(m_original_stick_world_position);
      });

  m_inputs.attach_to_root(m_soft_stick_input);
}

bim::axmol::widget::soft_stick::~soft_stick() = default;

void bim::axmol::widget::soft_stick::set_layout_on_the_left(bool left)
{
  m_on_the_left = left;

  if (isRunning())
    update_display();
}

bim::axmol::input::node_reference
bim::axmol::widget::soft_stick::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::soft_stick::onEnter()
{
  ax::Node::onEnter();

  m_soft_stick_input->reset();

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
  move_stick_to(m_original_stick_world_position);
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
  if (m_on_the_left)
    apply_bounds(m_context, m_controls->all_nodes, m_style_bounds_left);
  else
    apply_bounds(m_context, m_controls->all_nodes, m_style_bounds_right);

  const ax::Node& stick = *m_controls->stick;

  m_original_stick_world_position = stick.convertToWorldSpace(
      stick.getAnchorPoint() * stick.getContentSize());
}

void bim::axmol::widget::soft_stick::move_stick_to(
    const ax::Vec2& world_position)
{
  ax::Node& stick = *m_controls->stick;
  ax::Node& parent = *stick.getParent();
  const ax::Vec2 drag = m_soft_stick_input->drag();
  const ax::Vec2 parent_size = parent.getContentSize();

  if (drag == ax::Vec2(0, 0))
    {
      stick.setPosition(parent_size / 2);
      parent.setPosition(
          parent.getParent()->convertToNodeSpace(world_position));
      return;
    }

  const ax::Vec2 position_in_parent =
      parent.convertToNodeSpace(world_position);
  const ax::Vec2 half_parent_size = parent_size / 2;

  stick.setPosition(half_parent_size
                    + m_soft_stick_input->drag() * half_parent_size);

  const ax::Vec2 parent_position = parent.getPosition();
  const ax::Vec2 parent_anchor = parent.getAnchorPoint();
  ax::Vec2 new_parent_position = parent_position;
  const ax::Vec2 parent_zero = parent_position - parent_size * parent_anchor;

  const ax::Vec2 my_size = getContentSize();

  if (position_in_parent.x >= parent_size.x)
    // Slide the joystick to the right.
    new_parent_position.x +=
        std::min(position_in_parent.x - parent_size.x,
                 my_size.x - (parent_zero.x + parent_size.x));
  else if (position_in_parent.x <= 0)
    // Slide the joystick to the left.
    new_parent_position.x -= std::min(-position_in_parent.x, parent_zero.x);

  if (position_in_parent.y >= parent_size.y)
    // Slide the joystick upwards.
    new_parent_position.y +=
        std::min(position_in_parent.y - parent_size.y,
                 my_size.y - (parent_zero.y + parent_size.y));
  else if (position_in_parent.y <= 0)
    // Slide the joystick downwards.
    new_parent_position.y -= std::min(-position_in_parent.y, parent_zero.y);

  parent.setPosition(new_parent_position);
}
