// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/toggle.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/action/dynamic_factory.hpp>

#include <bim/axmol/input/observer/tap_observer.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <iscool/audio/default_effect_player.hpp>
#include <iscool/audio/loop_mode.hpp>
#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::widget::toggle::
#define x_widget_type_name controls
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/Action.h>

IMPLEMENT_SIGNAL(bim::axmol::widget::toggle, clicked, m_clicked);

bim_implement_widget(bim::axmol::widget::toggle);

bim::axmol::widget::toggle::toggle(const bim::axmol::widget::context& context,
                                   const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_container(ax::Node::create())
  , m_tap_observer(*this)
  , m_style_bounds(style.get_declaration_or_empty("bounds"))
  , m_style_off(style.get_declaration_or_empty("display.off"))
  , m_style_on(style.get_declaration_or_empty("display.on"))
  , m_action_pressed(context.action_factory.create(
        context.colors, style.get_declaration_or_empty("action.pressed")))
  , m_action_released(context.action_factory.create(
        context.colors, style.get_declaration_or_empty("action.released")))
  , m_sound(style.get_string("sound.click", ""))
  , m_bounds_dirty(true)
  , m_display_dirty(true)
  , m_is_on(false)
{
  m_container->setAnchorPoint(ax::Vec2(0.5, 0.5));
  m_container->setCascadeColorEnabled(true);
  m_container->setCascadeOpacityEnabled(true);

  m_tap_observer->connect_to_enter(
      [this]()
      {
        input_press();
      });
  m_tap_observer->connect_to_leave(
      [this]()
      {
        input_release();
      });
  m_tap_observer->connect_to_release(
      [this]()
      {
        toggle_state();
      });

  m_inputs.attach_to_root(m_tap_observer);
}

bim::axmol::widget::toggle::~toggle() = default;

bim::axmol::input::node_reference
bim::axmol::widget::toggle::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::toggle::onEnter()
{
  ax::Node::onEnter();

  if (m_bounds_dirty)
    update_bounds();

  if (m_display_dirty)
    update_display();
}

void bim::axmol::widget::toggle::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  m_container->setContentSize(size);
  m_container->setPosition(size / 2);

  update_bounds();
}

void bim::axmol::widget::toggle::enable(bool enabled)
{
  if (m_tap_observer->is_enabled() == enabled)
    return;

  m_tap_observer->enable(enabled);
}

void bim::axmol::widget::toggle::set_state(bool state)
{
  m_is_on = state;
  update_display();
}

bool bim::axmol::widget::toggle::get_state() const
{
  return m_is_on;
}

bool bim::axmol::widget::toggle::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  addChild(m_container.get());

  add_group_as_children(*m_container, m_controls->all_nodes);

  return true;
}

void bim::axmol::widget::toggle::input_press()
{
  m_container->stopAllActions();
  m_container->runAction(m_action_pressed.get());
}

void bim::axmol::widget::toggle::input_release()
{
  m_container->stopAllActions();
  m_container->runAction(m_action_released.get());
}

void bim::axmol::widget::toggle::toggle_state()
{
  input_release();

  if (!m_sound.empty())
    iscool::audio::play_effect(m_sound, iscool::audio::loop_mode::once);

  m_clicked();
}

void bim::axmol::widget::toggle::update_bounds()
{
  if (!isRunning())
    {
      m_bounds_dirty = true;
      return;
    }

  apply_bounds(m_context, m_controls->all_nodes, m_style_bounds);
  m_bounds_dirty = false;
}

void bim::axmol::widget::toggle::update_display()
{
  if (!isRunning())
    {
      m_display_dirty = true;
      return;
    }

  const iscool::style::declaration& style = m_is_on ? m_style_on : m_style_off;
  apply_display(m_context.style_cache, m_controls->all_nodes, style);

  m_display_dirty = false;
}
