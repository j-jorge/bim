// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/button.hpp>

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

#define x_widget_scope bim::axmol::widget::button::
#define x_widget_type_name controls
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/Action.h>
#include <axmol/2d/ActionInterval.h>

IMPLEMENT_SIGNAL(bim::axmol::widget::button, clicked, m_clicked);
IMPLEMENT_SIGNAL(bim::axmol::widget::button, pressed, m_pressed);

bim_implement_widget(bim::axmol::widget::button);

bim::axmol::widget::button::button(const bim::axmol::widget::context& context,
                                   const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_container(ax::Node::create())
  , m_tap_observer(*this)
  , m_style_bounds(style.get_declaration_or_empty("bounds"))
  , m_style_pressed(style.get_declaration_or_empty("display.pressed"))
  , m_style_released(style.get_declaration_or_empty("display.release"))
  , m_style_disabled(style.get_declaration_or_empty("display.disabled"))
  , m_sound(style.get_string("sound.click", ""))
  , m_bounds_dirty(true)
  , m_display_dirty(true)
  , m_is_pressed(false)
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
        click();
      });

  m_inputs.attach_to_root(m_tap_observer);

  const auto set_optional_action =
      [&, this](bim::axmol::ref_ptr<ax::Action>& action,
                const char* name) -> void
  {
    const iscool::optional<const iscool::style::declaration&> action_style =
        style.get_declaration(name);

    if (!action_style)
      return;

    bim::axmol::ref_ptr<ax::FiniteTimeAction> action_from_style =
        context.action_factory.create(context.colors, *action_style);

    action =
        ax::TargetedAction::create(m_container.get(), action_from_style.get());
  };

  set_optional_action(m_action_pressed, "action.pressed");
  set_optional_action(m_action_released, "action.released");
}

bim::axmol::widget::button::~button() = default;

bim::axmol::input::node_reference
bim::axmol::widget::button::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::button::onEnter()
{
  ax::Node::onEnter();

  if (m_bounds_dirty)
    update_bounds();

  if (m_display_dirty)
    update_display();
}

void bim::axmol::widget::button::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  m_container->setContentSize(size);
  m_container->setPosition(size / 2);

  update_bounds();
}

void bim::axmol::widget::button::enable(bool enabled)
{
  if (m_tap_observer->is_enabled() == enabled)
    return;

  m_tap_observer->enable(enabled);
  update_display();
}

void bim::axmol::widget::button::cancel_on_swipe(bool v)
{
  m_tap_observer->cancel_on_swipe(v);
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

void bim::axmol::widget::button::input_press()
{
  m_is_pressed = true;
  update_display();

  m_action_runner.stop();

  if (m_action_pressed)
    m_action_runner.run(*m_action_pressed);

  m_pressed();
}

void bim::axmol::widget::button::input_release()
{
  m_is_pressed = false;
  update_display();

  m_action_runner.stop();

  if (m_action_released)
    m_action_runner.run(*m_action_released);
}

void bim::axmol::widget::button::click()
{
  input_release();

  if (!m_sound.empty())
    iscool::audio::play_effect(m_sound, iscool::audio::loop_mode::once);

  m_clicked();
}

void bim::axmol::widget::button::update_bounds()
{
  if (!isRunning())
    {
      m_bounds_dirty = true;
      return;
    }

  apply_bounds(m_context, m_controls->all_nodes, m_style_bounds);
  m_bounds_dirty = false;
}

void bim::axmol::widget::button::update_display()
{
  if (!isRunning())
    {
      m_display_dirty = true;
      return;
    }

  const iscool::style::declaration* style;

  if (m_is_pressed)
    style = &m_style_pressed;
  else if (m_tap_observer->is_enabled())
    style = &m_style_released;
  else
    style = &m_style_disabled;

  apply_display(m_context.style_cache, m_controls->all_nodes, *style);

  m_display_dirty = false;
}
