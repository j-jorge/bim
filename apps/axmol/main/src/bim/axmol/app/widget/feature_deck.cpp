// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/widget/feature_deck.hpp>

#include <bim/axmol/app/game_feature_sprite_frame.hpp>
#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/2d/Sprite.h>

#define x_widget_scope bim::axmol::app::feature_deck::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Sprite, slot_0) x_widget(ax::Sprite, slot_1)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <cassert>

bim_implement_widget(bim::axmol::app::feature_deck);

bim::axmol::app::feature_deck::feature_deck(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_style_bounds{ &*style.get_declaration("bounds.0"),
                    &*style.get_declaration("bounds.1"),
                    &*style.get_declaration("bounds.2") }
  , m_style_display{ &*style.get_declaration("display.0"),
                     &*style.get_declaration("display.1"),
                     &*style.get_declaration("display.2") }
  , m_slot{ m_controls->slot_0, m_controls->slot_1 }
  , m_feature_flags{}
  , m_slot_count(0)
  , m_dirty(false)
{}

bim::axmol::app::feature_deck::~feature_deck() = default;

void bim::axmol::app::feature_deck::features(bim::game::feature_flags flags)
{
  if (flags == m_feature_flags)
    return;

  m_feature_flags = flags;

  std::size_t i = 0;
  for (const bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    if (!!(flags & f))
      {
        assert(i < bim::app::g_game_feature_slot_count);
        m_slot[i]->setSpriteFrame(&game_feature_sprite_frame(f));
        ++i;
      }

  m_slot_count = i;

  update_display();
}

void bim::axmol::app::feature_deck::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  update_display();
}

bool bim::axmol::app::feature_deck::init()
{
  if (!ax::Node::init())
    return false;

  bim::axmol::widget::add_group_as_children(*this, m_controls->all_nodes);

  return true;
}

void bim::axmol::app::feature_deck::onEnter()
{
  ax::Node::onEnter();

  if (m_dirty)
    update_display();
}

void bim::axmol::app::feature_deck::update_display()
{
  if (!isRunning())
    {
      m_dirty = true;
      return;
    }

  m_dirty = false;

  bim::axmol::widget::apply_display(m_context.style_cache,
                                    m_controls->all_nodes,
                                    *m_style_display[m_slot_count]);
  bim::axmol::widget::apply_bounds(m_context, m_controls->all_nodes,
                                   *m_style_bounds[m_slot_count]);
}
