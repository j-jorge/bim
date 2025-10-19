// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/widget/game_feature_button.hpp>

#include <bim/axmol/app/game_feature_sprite_frame.hpp>
#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/colour_chart.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/Sprite.h>

#define x_widget_scope bim::axmol::app::game_feature_button::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Sprite, feature_icon) x_widget(ax::Label, price_label)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/i18n/numeric.hpp>

#include <cassert>

bim_implement_widget(bim::axmol::app::game_feature_button);

bim::axmol::app::game_feature_button::game_feature_button(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_container(ax::Node::create())
  , m_behavior(context, style, *m_container, m_controls->all_nodes)
  , m_style_available(*style.get_declaration("display.available"))
  , m_style_unavailable(*style.get_declaration("display.unavailable"))
  , m_style_feature(*style.get_declaration("display.feature"))
  , m_style_no_feature(*style.get_declaration("display.no-feature"))
  , m_color_affordable(
        context.colors.to_color_4b(*style.get_string("color.affordable")))
  , m_color_unaffordable(
        context.colors.to_color_4b(*style.get_string("color.unaffordable")))
{
  m_container->setName("container");
  m_container->setAnchorPoint(ax::Vec2(0.5, 0.5));
  m_container->setCascadeColorEnabled(true);
  m_container->setCascadeOpacityEnabled(true);
}

bim::axmol::app::game_feature_button::~game_feature_button() = default;

iscool::signals::connection
bim::axmol::app::game_feature_button::connect_to_clicked(
    std::function<void()> f) const
{
  return m_behavior.connect_to_clicked(std::move(f));
}

bim::axmol::input::node_reference
bim::axmol::app::game_feature_button::input_node() const
{
  return m_behavior.input_node();
}

void bim::axmol::app::game_feature_button::onEnter()
{
  ax::Node::onEnter();
  m_behavior.on_enter();
}

void bim::axmol::app::game_feature_button::onExit()
{
  m_behavior.on_exit();
  ax::Node::onExit();
}

void bim::axmol::app::game_feature_button::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  m_container->setContentSize(size);
  m_container->setPosition(size / 2);

  m_behavior.container_size_changed();
}

void bim::axmol::app::game_feature_button::feature(bim::game::feature_flags f)
{
  if (f == bim::game::feature_flags{})
    {
      bim::axmol::widget::apply_display(
          m_context.style_cache, m_controls->all_nodes, m_style_no_feature);
      return;
    }

  m_controls->feature_icon->setSpriteFrame(&game_feature_sprite_frame(f));

  bim::axmol::widget::apply_display(m_context.style_cache,
                                    m_controls->all_nodes, m_style_feature);
}

void bim::axmol::app::game_feature_button::available(bool a)
{
  bim::axmol::widget::apply_display(
      m_context.style_cache, m_controls->all_nodes,
      a ? m_style_available : m_style_unavailable);
}

void bim::axmol::app::game_feature_button::price(int p)
{
  m_controls->price_label->setString(iscool::i18n::numeric::to_string(p));
}

void bim::axmol::app::game_feature_button::affordable(bool a)
{
  m_controls->price_label->setTextColor(a ? m_color_affordable
                                          : m_color_unaffordable);
}

bool bim::axmol::app::game_feature_button::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  addChild(m_container.get());

  bim::axmol::widget::add_group_as_children(*m_container,
                                            m_controls->all_nodes);

  return true;
}
