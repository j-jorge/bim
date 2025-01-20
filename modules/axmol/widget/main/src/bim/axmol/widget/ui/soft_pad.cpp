// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/soft_pad.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/input/observer/soft_pad.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Sprite.h>

#define x_widget_scope bim::axmol::widget::soft_pad::
#define x_widget_type_name controls
#define x_widget_controls
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::widget::soft_pad, pressed, m_pressed);

bim_implement_widget(bim::axmol::widget::soft_pad);

bim::axmol::widget::soft_pad::soft_pad(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_soft_pad_input(*this)
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_style_up(*style.get_declaration("display.up"))
  , m_style_down(*style.get_declaration("display.down"))
  , m_style_left(*style.get_declaration("display.left"))
  , m_style_right(*style.get_declaration("display.right"))
  , m_style_off(*style.get_declaration("display.off"))
{
  m_soft_pad_input->connect_to_changed(
      [this]()
      {
        if (update_display())
          m_pressed();
      });

  m_inputs.attach_to_root(m_soft_pad_input);
}

bim::axmol::widget::soft_pad::~soft_pad() = default;

bim::axmol::input::node_reference
bim::axmol::widget::soft_pad::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::soft_pad::onEnter()
{
  ax::Node::onEnter();

  m_soft_pad_input->reset();

  update_display();
}

void bim::axmol::widget::soft_pad::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  update_display();
}

void bim::axmol::widget::soft_pad::enable(bool enabled)
{
  if (m_soft_pad_input->is_enabled() == enabled)
    return;

  m_soft_pad_input->enable(enabled);
  update_display();
}

ax::Vec2 bim::axmol::widget::soft_pad::direction() const
{
  return ax::Vec2(m_soft_pad_input->horizontal(),
                  m_soft_pad_input->vertical());
}

bool bim::axmol::widget::soft_pad::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}

bool bim::axmol::widget::soft_pad::update_display()
{
  apply_bounds(m_context, m_controls->all_nodes, m_style_bounds);

  if (m_soft_pad_input->horizontal() > 0)
    apply_display(m_context.style_cache, m_controls->all_nodes, m_style_right);
  else if (m_soft_pad_input->horizontal() < 0)
    apply_display(m_context.style_cache, m_controls->all_nodes, m_style_left);
  else if (m_soft_pad_input->vertical() > 0)
    apply_display(m_context.style_cache, m_controls->all_nodes, m_style_up);
  else if (m_soft_pad_input->vertical() < 0)
    apply_display(m_context.style_cache, m_controls->all_nodes, m_style_down);
  else
    {
      apply_display(m_context.style_cache, m_controls->all_nodes, m_style_off);
      return false;
    }

  return true;
}
