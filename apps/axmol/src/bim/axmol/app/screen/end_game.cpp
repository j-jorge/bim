// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/end_game.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/net/exchange/game_launch_event.hpp>

#include <bim/game/contest_result.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::end_game::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, revenge_button)                        \
      x_widget(bim::axmol::widget::button, quit_button)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::end_game, quit, m_quit);
IMPLEMENT_SIGNAL(bim::axmol::app::end_game, revenge, m_revenge);

bim::axmol::app::end_game::end_game(const context& context,
                                    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_style_draw(*style.get_declaration("display.draw"))
  , m_style_win(*style.get_declaration("display.win"))
  , m_style_lose(*style.get_declaration("display.lose"))
{
  m_inputs.push_back(m_controls->revenge_button->input_node());

  m_controls->revenge_button->connect_to_clicked(
      [this]()
      {
        m_revenge();
      });

  m_inputs.push_back(m_controls->quit_button->input_node());

  m_controls->quit_button->connect_to_clicked(
      [this]()
      {
        m_quit();
      });
}

bim::axmol::app::end_game::~end_game() = default;

bim::axmol::input::node_reference bim::axmol::app::end_game::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::end_game::nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::end_game::game_started(
    const bim::net::game_launch_event& event)
{
  m_player_index = event.player_index;
}

void bim::axmol::app::end_game::displaying(
    const bim::game::contest_result& result)
{
  const iscool::style::declaration* style;

  if (!result.has_a_winner())
    style = &m_style_draw;
  else if (result.winning_player() == m_player_index)
    style = &m_style_win;
  else
    style = &m_style_lose;

  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, *style);
}

void bim::axmol::app::end_game::displayed()
{}

void bim::axmol::app::end_game::closing()
{}
