#include <bim/axmol/app/screen/matchmaking.hpp>

#include <bim/axmol/widget/apply_actions.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/net/exchange/new_game_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>

#include <fmt/format.h>

#define x_widget_scope bim::axmol::app::matchmaking::
#define x_widget_type_name controls
#define x_widget_controls x_widget(bim::axmol::widget::button, ready_button)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::matchmaking, start_game, m_start_game);

ic_implement_state_monitor(bim::axmol::app::matchmaking, m_monitor, off,
                           ((off)((waiting)))                               //
                           ((waiting)((match_2)(match_3)(match_4)))         //
                           ((match_2)((waiting)(match_3)(match_4)(launch))) //
                           ((match_3)((waiting)(match_2)(match_4)(launch))) //
                           ((match_4)((waiting)(match_2)(match_3)(launch))) //
                           ((launch)((off))));

bim::axmol::app::matchmaking::matchmaking(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_new_game(new bim::net::new_game_exchange(
        m_context.get_session_handler()->message_stream()))
  , m_style_displaying(*style.get_declaration("display.displaying"))
  , m_action_displaying(*style.get_declaration("actions.displaying"))
  , m_action_wait(*style.get_declaration("actions.wait"))
  , m_action_2_players(*style.get_declaration("actions.2-players"))
  , m_action_3_players(*style.get_declaration("actions.3-players"))
  , m_action_4_players(*style.get_declaration("actions.4-players"))
{
  m_inputs.push_back(m_controls->ready_button->input_node());

  m_controls->ready_button->connect_to_clicked(
      [this]()
      {
        accept_game();
      });
}

bim::axmol::app::matchmaking::~matchmaking() = default;

bim::axmol::input::node_reference
bim::axmol::app::matchmaking::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::matchmaking::nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::matchmaking::displaying()
{
  m_monitor->set_waiting_state();

  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_displaying);

  run_actions(m_main_actions, m_action_displaying);
  run_actions(m_state_actions, m_action_wait);

  m_controls->ready_button->enable(true);
}

void bim::axmol::app::matchmaking::displayed()
{
  m_game_proposal_connection = m_new_game->connect_to_game_proposal(
      [this](unsigned player_count)
      {
        update_display_with_game_proposal(player_count);
      });
  m_launch_connection = m_new_game->connect_to_launch_game(
      [this](const bim::net::game_launch_event& event)
      {
        launch_game(event);
      });

  assert(m_context.get_session_handler()->connected());
  m_new_game->start(m_context.get_session_handler()->session_id());
}

void bim::axmol::app::matchmaking::closing()
{
  m_monitor->set_off_state();
  m_game_proposal_connection.disconnect();
  m_new_game->stop();
}

void bim::axmol::app::matchmaking::update_display_with_game_proposal(
    unsigned player_count)
{
  const iscool::style::declaration* action;

  printf("%d players\n", player_count);

  switch (player_count)
    {
    case 1:
      if (m_monitor->is_waiting_state())
        return;
      m_monitor->set_waiting_state();
      action = &m_action_wait;
      break;
    case 2:
      if (m_monitor->is_match_2_state())
        return;
      m_monitor->set_match_2_state();
      action = &m_action_2_players;
      break;
    case 3:
      if (m_monitor->is_match_3_state())
        return;
      m_monitor->set_match_3_state();
      action = &m_action_3_players;
      break;
    default:
      if (m_monitor->is_match_4_state())
        return;
      m_monitor->set_match_4_state();
      action = &m_action_4_players;
      break;
    }

  run_actions(m_state_actions, *action);
}

void bim::axmol::app::matchmaking::run_actions(
    bim::axmol::action::runner& runner,
    const iscool::style::declaration& style) const
{
  runner.stop();

  bim::axmol::widget::apply_actions(runner, m_context.get_widget_context(),
                                    m_controls->all_nodes, style);
}

void bim::axmol::app::matchmaking::accept_game()
{
  m_monitor->set_launch_state();
  m_controls->ready_button->enable(false);

  m_game_proposal_connection.disconnect();
  m_new_game->accept();
}

void bim::axmol::app::matchmaking::launch_game(
    const bim::net::game_launch_event& event)
{
  m_launch_connection.disconnect();
  m_start_game(event);
}
