// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/end_game.hpp>

#include <bim/axmol/app/part/wallet.hpp>

#include <bim/axmol/widget/apply_actions.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/factory/node.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/app/analytics/button_clicked.hpp>
#include <bim/app/config.hpp>

#include <bim/net/exchange/game_launch_event.hpp>

#include <bim/game/contest_result.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::end_game::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, revenge_button)                        \
      x_widget(bim::axmol::widget::button, quit_button)                       \
          x_widget(ax::Node, reward_reference)                                \
              x_widget(ax::Label, reward_label)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/i18n/numeric.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::end_game, quit, m_quit);
IMPLEMENT_SIGNAL(bim::axmol::app::end_game, revenge, m_revenge);

bim::axmol::app::end_game::end_game(const context& context,
                                    const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_wallet(new wallet(context, *style.get_declaration("wallet")))
  , m_action_draw(*style.get_declaration("actions.draw"))
  , m_action_win(*style.get_declaration("actions.win"))
  , m_action_lose(*style.get_declaration("actions.lose"))
{
  m_all_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_wallet->display_nodes());

  m_inputs.push_back(m_controls->revenge_button->input_node());

  m_controls->revenge_button->connect_to_clicked(
      [this]()
      {
        dispatch_revenge();
      });

  m_inputs.push_back(m_controls->quit_button->input_node());

  m_controls->quit_button->connect_to_clicked(
      [this]()
      {
        dispatch_quit();
      });
}

bim::axmol::app::end_game::~end_game() = default;

bim::axmol::input::node_reference bim::axmol::app::end_game::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::end_game::display_nodes() const
{
  return m_all_nodes;
}

void bim::axmol::app::end_game::game_started(
    const bim::net::game_launch_event& event)
{
  m_player_index = event.player_index;
  m_wallet->enter();
}

void bim::axmol::app::end_game::displaying(
    const bim::game::contest_result& result)
{
  const iscool::style::declaration* action_style;
  int reward;

  if (!result.has_a_winner())
    {
      action_style = &m_action_draw;
      reward = m_context.get_config()->coins_per_draw;
    }
  else if (result.winning_player() == m_player_index)
    {
      action_style = &m_action_win;
      reward = m_context.get_config()->coins_per_victory;
    }
  else
    {
      action_style = &m_action_lose;
      reward = m_context.get_config()->coins_per_defeat;
    }

  m_controls->reward_label->setString(
      "+" + iscool::i18n::numeric::to_string(reward));
  m_main_actions.stop();

  bim::axmol::widget::apply_actions(m_main_actions,
                                    m_context.get_widget_context(),
                                    m_all_nodes, *action_style);
}

void bim::axmol::app::end_game::displayed()
{
  m_wallet->animate_cash_flow(
      m_controls->reward_reference->convertToWorldSpace(
          m_controls->reward_reference->getContentSize() / 2));
}

void bim::axmol::app::end_game::closing()
{
  m_main_actions.stop();
}

void bim::axmol::app::end_game::dispatch_revenge()
{
  bim::app::button_clicked(*m_context.get_analytics(), "revenge", "end-game");
  m_revenge();
}

void bim::axmol::app::end_game::dispatch_quit()
{
  bim::app::button_clicked(*m_context.get_analytics(), "quit", "end-game");
  m_quit();
}
