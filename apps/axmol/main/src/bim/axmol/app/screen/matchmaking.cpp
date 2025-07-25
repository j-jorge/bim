// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/matchmaking.hpp>

#include <bim/axmol/app/config.hpp>
#include <bim/axmol/app/feature_deck.hpp>
#include <bim/axmol/app/matchmaking_wait_message.hpp>
#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/preference/feature_flags.hpp>
#include <bim/axmol/app/preference/wallet.hpp>

#include <bim/axmol/widget/apply_actions.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>

#include <bim/net/exchange/new_game_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>
#include <axmol/base/EventKeyboard.h>

#include <fmt/format.h>

#define x_widget_scope bim::axmol::app::matchmaking::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, ready_button)                          \
      x_widget(bim::axmol::widget::button, back_button)                       \
          x_widget(ax::Label, feature_description)                            \
              x_widget(ax::Label, wait_message)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::matchmaking, start_game, m_start_game);
IMPLEMENT_SIGNAL(bim::axmol::app::matchmaking, back, m_back);

ic_implement_state_monitor(bim::axmol::app::matchmaking,
                           m_player_count_monitor, off,
                           ((off)((waiting)))                            //
                           ((waiting)((match_2)(match_3)(match_4)(off))) //
                           ((match_2)((waiting)(match_3)(match_4)(off))) //
                           ((match_3)((waiting)(match_2)(match_4)(off))) //
                           ((match_4)((waiting)(match_2)(match_3)(off))));
ic_implement_state_monitor(bim::axmol::app::matchmaking, m_launch_monitor, off,
                           ((off)((launch)(off))) //
                           ((launch)((off))));

bim::axmol::app::matchmaking::matchmaking(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_wallet(new wallet(context, *style.get_declaration("wallet")))
  , m_new_game(new bim::net::new_game_exchange(
        m_context.get_session_handler()->message_stream()))
  , m_feature_deck(
        new feature_deck(m_context, *style.get_declaration("feature-deck")))
  , m_wait_message(new matchmaking_wait_message())
  , m_style_displaying(*style.get_declaration("display.displaying"))
  , m_action_displaying(*style.get_declaration("actions.displaying"))
  , m_action_wait(*style.get_declaration("actions.wait"))
  , m_action_2_players(*style.get_declaration("actions.2-players"))
  , m_action_3_players(*style.get_declaration("actions.3-players"))
  , m_action_4_players(*style.get_declaration("actions.4-players"))
{
  m_all_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_feature_deck->display_nodes());
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_wallet->display_nodes());

  m_inputs.push_back(m_feature_deck->input_node());
  m_inputs.push_back(m_controls->ready_button->input_node());
  m_inputs.push_back(m_escape);
  m_inputs.push_back(m_controls->back_button->input_node());

  m_controls->ready_button->connect_to_clicked(
      [this]()
      {
        accept_game();
      });

  m_escape->connect_to_released(
      [this]()
      {
        dispatch_back();
      });
  m_controls->back_button->connect_to_clicked(
      [this]()
      {
        dispatch_back();
      });

  m_feature_deck->connect_to_clicked(
      [this](bim::game::feature_flags f) -> void
      {
        feature_flag_clicked(f);
      });

  m_wait_message->connect_to_updated(
      [this](std::string_view m) -> void
      {
        m_controls->wait_message->setString(m);
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
  return m_all_nodes;
}

void bim::axmol::app::matchmaking::attached()
{
  m_wallet->attached();
}

void bim::axmol::app::matchmaking::displaying()
{
  m_wallet->enter();

  m_player_count_monitor->set_waiting_state();

  bim::axmol::widget::apply_display(m_context.get_widget_context().style_cache,
                                    m_controls->all_nodes, m_style_displaying);

  m_feature_deck->displaying();

  run_actions(m_main_actions, m_action_displaying);
  run_actions(m_state_actions, m_action_wait);

  show_default_feature_message();
  m_controls->ready_button->enable(true);
  m_controls->wait_message->setString("");
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
  m_main_actions.stop();
  m_state_actions.stop();

  m_wait_message->stop();
  m_player_count_monitor->set_off_state();
  m_launch_monitor->set_off_state();

  m_game_proposal_connection.disconnect();
  m_new_game->stop();
}

void bim::axmol::app::matchmaking::update_display_with_game_proposal(
    unsigned player_count)
{
  // If we tried to launch the game but we are back to a single player
  // (i.e. the local player), then the request has failed. Let's start a new
  // request.
  if ((player_count == 1) && m_launch_monitor->is_launch_state())
    {
      m_launch_monitor->set_off_state();
      m_new_game->stop();
      m_new_game->start(m_context.get_session_handler()->session_id());
      m_controls->ready_button->enable(true);
    }

  const iscool::style::declaration* action;

  if (player_count == 1)
    m_wait_message->start();
  else
    m_wait_message->pause();

  switch (player_count)
    {
    case 1:
      if (m_player_count_monitor->is_waiting_state())
        return;
      m_player_count_monitor->set_waiting_state();
      action = &m_action_wait;
      break;
    case 2:
      if (m_player_count_monitor->is_match_2_state())
        return;
      m_player_count_monitor->set_match_2_state();
      action = &m_action_2_players;
      m_wait_message->pause();
      break;
    case 3:
      if (m_player_count_monitor->is_match_3_state())
        return;
      m_player_count_monitor->set_match_3_state();
      action = &m_action_3_players;
      break;
    default:
      if (m_player_count_monitor->is_match_4_state())
        return;
      m_player_count_monitor->set_match_4_state();
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
  m_launch_monitor->set_launch_state();
  m_controls->ready_button->enable(false);

  const bim::game::feature_flags features =
      enabled_feature_flags(*m_context.get_local_preferences());
  m_new_game->accept(features);
}

void bim::axmol::app::matchmaking::launch_game(
    const bim::net::game_launch_event& event)
{
  m_game_proposal_connection.disconnect();
  m_launch_connection.disconnect();
  m_start_game(event);
}

void bim::axmol::app::matchmaking::dispatch_back() const
{
  if (!m_player_count_monitor->is_waiting_state())
    return;

  m_back();
}

void bim::axmol::app::matchmaking::feature_flag_clicked(
    bim::game::feature_flags f) const
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  bim::game::feature_flags available_features =
      available_feature_flags(preferences);
  bim::game::feature_flags enabled_features =
      enabled_feature_flags(preferences);

  if (!(available_features & f))
    {
      const std::int64_t coins = coins_balance(preferences);
      const std::int16_t price = m_context.get_config()->game_feature_price[f];

      if (price <= coins)
        {
          consume_coins(preferences, price);
          m_wallet->animate_cash_flow();

          available_features |= f;
          available_feature_flags(preferences, available_features);

          enabled_features |= f;
          enabled_feature_flags(preferences, enabled_features);

          m_feature_deck->purchased(f);
          m_feature_deck->activated(f);

          show_feature_message(f);
        }
      else
        m_controls->feature_description->setString(
            ic_gettext("You need more coins to purchase this item!"));
    }
  else
    {
      enabled_features = enabled_features ^ f;
      enabled_feature_flags(preferences, enabled_features);

      if (!(enabled_features & f))
        {
          m_feature_deck->deactivated(f);
          show_default_feature_message();
        }
      else
        {
          m_feature_deck->activated(f);
          show_feature_message(f);
        }
    }
}

void bim::axmol::app::matchmaking::show_default_feature_message() const
{
  m_controls->feature_description->setString(
      ic_gettext("Customize your experience below!"));
}

void bim::axmol::app::matchmaking::show_feature_message(
    bim::game::feature_flags f) const
{
  const char* message = "";

  switch (f)
    {
    case bim::game::feature_flags::falling_blocks:
      message =
          ic_gettext("Falling blocks reduce the arena after two minutes!");
      break;
    case bim::game::feature_flags::fog_of_war:
      message = ic_gettext(
          "A thick fog covers the arena. You can't see were you did not go!");
      break;
    case bim::game::feature_flags::invisibility:
      message = ic_gettext("Find the invisibility power up to disappear from "
                           "the screen of all other players!");
      break;
    }

  m_controls->feature_description->setString(message);
}
