// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/widget/player.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/animation/animation.hpp>
#include <bim/axmol/widget/animation/animation_cache.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/game/component/animation_state.hpp>

#include <axmol/2d/Sprite.h>

#define x_widget_scope bim::axmol::app::player::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Sprite, body)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <bim/game/context/player_animations.hpp>

#include <cassert>

bim_implement_widget(bim::axmol::app::player);

bim::axmol::app::player::player(const bim::axmol::widget::context& context,
                                const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
{
  setCascadeOpacityEnabled(true);
}

bim::axmol::app::player::~player() = default;

void bim::axmol::app::player::configure(
    const bim::axmol::widget::animation_cache& animation_cache,
    const bim::game::player_animations& player_animations,
    std::uint8_t player_index)
{
  const std::string suffix = '_' + std::to_string(player_index + 1);
  std::string name;

  auto add_animation = [&](bim::game::animation_id id,
                           std::string_view n) mutable -> void
  {
    name = n;
    name += suffix;
    m_animations[id] = &animation_cache.get(name);
  };

#define add_animation_n(n) add_animation(player_animations.n, #n)

  add_animation_n(idle_down);
  add_animation_n(idle_left);
  add_animation_n(idle_right);
  add_animation_n(idle_up);

  add_animation_n(walk_down);
  add_animation_n(walk_left);
  add_animation_n(walk_right);
  add_animation_n(walk_up);

  add_animation_n(die);
#undef add_animation_n

  m_animations[player_animations.burn] = &animation_cache.get("burn");
}

void bim::axmol::app::player::set_animation(
    const bim::game::animation_state& state)
{
  assert(m_animations.find(state.model) != m_animations.end());

  m_animations.find(state.model)->second->apply(*m_controls->body, state);
}

void bim::axmol::app::player::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  bim::axmol::widget::apply_bounds(m_context, m_controls->all_nodes,
                                   m_style_bounds);
}

bool bim::axmol::app::player::init()
{
  if (!ax::Node::init())
    return false;

  bim::axmol::widget::add_group_as_children(*this, m_controls->all_nodes);

  // We need a default size to be properly scaled in the arena.
  setContentSize(m_controls->body->getContentSize());

  return true;
}
