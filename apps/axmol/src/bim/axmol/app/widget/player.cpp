// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/widget/player.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/2d/Sprite.h>

#define x_widget_scope bim::axmol::app::player::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Sprite, front) x_widget(ax::Sprite, back)                      \
      x_widget(ax::Sprite, left) x_widget(ax::Sprite, right)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <bim/game/component/player_direction.hpp>

bim_implement_widget(bim::axmol::app::player);

bim::axmol::app::player::player(const bim::axmol::widget::context& context,
                                const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
{}

bim::axmol::app::player::~player() = default;

void bim::axmol::app::player::set_direction(bim::game::player_direction d)
{
  m_controls->front->setVisible(d == bim::game::player_direction::down);
  m_controls->back->setVisible(d == bim::game::player_direction::up);
  m_controls->left->setVisible(d == bim::game::player_direction::left);
  m_controls->right->setVisible(d == bim::game::player_direction::right);
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
  setContentSize(m_controls->front->getContentSize());

  return true;
}
