// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/button_behavior.hpp>

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
#include <iscool/style/declaration.hpp>

#include <axmol/2d/Action.h>
#include <axmol/2d/ActionInterval.h>

IMPLEMENT_SIGNAL(bim::axmol::widget::button_behavior, clicked, m_clicked);
IMPLEMENT_SIGNAL(bim::axmol::widget::button_behavior, pressed, m_pressed);

bim::axmol::widget::button_behavior::button_behavior(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style, ax::Node& container,
    const named_node_group& all_nodes)
  : m_context(context)
  , m_all_nodes(all_nodes)
  , m_container(container)
  , m_tap_observer(container)
  , m_style_bounds(style.get_declaration_or_empty("bounds"))
  , m_style_pressed(style.get_declaration_or_empty("display.pressed"))
  , m_style_released(style.get_declaration_or_empty("display.release"))
  , m_style_disabled(style.get_declaration_or_empty("display.disabled"))
  , m_sound(style.get_string("sound.click", ""))
  , m_bounds_dirty(true)
  , m_display_dirty(true)
  , m_is_pressed(false)
{
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

    action = ax::TargetedAction::create(&m_container, action_from_style.get());
  };

  set_optional_action(m_action_pressed, "action.pressed");
  set_optional_action(m_action_released, "action.released");
}

bim::axmol::widget::button_behavior::~button_behavior() = default;

bim::axmol::input::node_reference
bim::axmol::widget::button_behavior::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::button_behavior::on_enter()
{
  if (m_bounds_dirty)
    update_bounds();

  if (m_display_dirty)
    update_display();
}

void bim::axmol::widget::button_behavior::on_exit()
{
  if (m_is_pressed && m_action_released)
    {
      m_is_pressed = false;
      m_action_runner.stop();
      m_action_runner.run_complete(*m_action_released);
    }
}

void bim::axmol::widget::button_behavior::container_size_changed()
{
  update_bounds();
}

void bim::axmol::widget::button_behavior::enable(bool enabled)
{
  if (m_tap_observer->is_enabled() == enabled)
    return;

  m_tap_observer->enable(enabled);
  update_display();
}

void bim::axmol::widget::button_behavior::cancel_on_swipe(bool v)
{
  m_tap_observer->cancel_on_swipe(v);
}

void bim::axmol::widget::button_behavior::input_press()
{
  m_is_pressed = true;
  update_display();

  m_action_runner.stop();

  if (m_action_pressed)
    m_action_runner.run(*m_action_pressed);

  m_pressed();
}

void bim::axmol::widget::button_behavior::input_release()
{
  m_is_pressed = false;
  update_display();

  m_action_runner.stop();

  if (m_action_released)
    m_action_runner.run(*m_action_released);
}

void bim::axmol::widget::button_behavior::click()
{
  input_release();

  if (!m_sound.empty())
    iscool::audio::play_effect(m_sound, iscool::audio::loop_mode::once);

  m_clicked();
}

void bim::axmol::widget::button_behavior::update_bounds()
{
  if (!m_container.isRunning())
    {
      m_bounds_dirty = true;
      return;
    }

  apply_bounds(m_context, m_all_nodes, m_style_bounds);
  m_bounds_dirty = false;
}

void bim::axmol::widget::button_behavior::update_display()
{
  if (!m_container.isRunning())
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

  apply_display(m_context.style_cache, m_all_nodes, *style);

  m_display_dirty = false;
}
