// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/widget/game_feature_button.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/toggle.hpp>

#include <bim/axmol/colour_chart.hpp>

#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::game_feature_button::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::toggle, toggle) x_widget(ax::Label, price_label)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/i18n/numeric.hpp>

#include <cassert>

bim_implement_widget(bim::axmol::app::game_feature_button);

bim::axmol::app::game_feature_button::game_feature_button(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_style_available(*style.get_declaration("display.available"))
  , m_style_unavailable(*style.get_declaration("display.unavailable"))
  , m_color_affordable(
        context.colors.to_color_4b(*style.get_string("color.affordable")))
  , m_color_unaffordable(
        context.colors.to_color_4b(*style.get_string("color.unaffordable")))
{
  setCascadeOpacityEnabled(true);
}

bim::axmol::app::game_feature_button::~game_feature_button() = default;

iscool::signals::connection
bim::axmol::app::game_feature_button::connect_to_clicked(
    std::function<void()> f) const
{
  return m_controls->toggle->connect_to_clicked(std::move(f));
}

bim::axmol::input::node_reference
bim::axmol::app::game_feature_button::input_node() const
{
  return m_controls->toggle->input_node();
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

void bim::axmol::app::game_feature_button::active(bool a)
{
  m_controls->toggle->set_state(a);
}

void bim::axmol::app::game_feature_button::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  bim::axmol::widget::apply_bounds(m_context, m_controls->all_nodes,
                                   m_style_bounds);
}

bool bim::axmol::app::game_feature_button::init()
{
  if (!ax::Node::init())
    return false;

  bim::axmol::widget::add_group_as_children(*this, m_controls->all_nodes);

  return true;
}
