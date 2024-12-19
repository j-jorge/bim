// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/exchange/game_update_exchange.hpp>

#include <bim/net/message/game_over.hpp>
#include <bim/net/message/game_update_from_server.hpp>
#include <bim/net/message/ready.hpp>
#include <bim/net/message/start.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest_result.hpp>

#include <bim/assume.hpp>

#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

ic_implement_state_monitor(bim::net::game_update_exchange, m_monitor, idle,
                           ((idle)((start)))     //
                           ((start)((play)))     //
                           ((play)((game_over))) //
                           ((game_over)()));

IMPLEMENT_SIGNAL(bim::net::game_update_exchange, started, m_started);
IMPLEMENT_SIGNAL(bim::net::game_update_exchange, updated, m_updated);
IMPLEMENT_SIGNAL(bim::net::game_update_exchange, game_over, m_game_over);

bim::net::game_update_exchange::game_update_exchange(
    iscool::net::message_channel& channel, std::uint8_t player_count)
  : m_message_channel(channel)
  , m_player_count(player_count)
{
  m_channel_signal_connection = m_message_channel.connect_to_message(std::bind(
      &game_update_exchange::deserialize, this, std::placeholders::_2));
}

bim::net::game_update_exchange::~game_update_exchange() = default;

void bim::net::game_update_exchange::start()
{
  m_monitor->set_start_state();

  m_client_out_message = bim::net::ready().build_message();
  send();
}

void bim::net::game_update_exchange::push(
    const bim::game::player_action& action)
{
  m_action_queue.emplace_back(action);

  if (append_to_current_update(action))
    m_client_out_message = m_current_update.build_message();

  if (!m_send_connection.connected())
    send();
}

void bim::net::game_update_exchange::deserialize(
    const iscool::net::message& message)
{
  switch (message.get_type())
    {
    case message_type::start:
      if (m_monitor->is_start_state())
        dispatch_start();
      break;
    case message_type::game_update_from_server:
      if (m_monitor->is_play_state())
        confirm_game_tick(message);
      break;
    case message_type::game_over:
      if (m_monitor->is_play_state())
        dispatch_game_over(message);
      break;
    }
}

void bim::net::game_update_exchange::dispatch_start()
{
  m_monitor->set_play_state();

  m_current_update.from_tick = 0;

  m_current_update.actions.clear();
  m_current_update.actions.reserve(32);

  m_send_connection.disconnect();

  m_started();
}

bool bim::net::game_update_exchange::append_to_current_update(
    const bim::game::player_action& action)
{
  constexpr int max_ticks_in_update_message = 64;

  // If the message would become too large when adding the provided action,
  // then keep it for later. We need the server to confirm the previous actions
  // first.
  if (m_current_update.actions.size() >= max_ticks_in_update_message)
    return false;

  m_current_update.actions.push_back(action);
  return true;
}

void bim::net::game_update_exchange::send()
{
  m_message_channel.send(m_client_out_message);

  m_send_connection = iscool::schedule::delayed_call(
      std::bind(&game_update_exchange::send, this),
      std::chrono::milliseconds(15));
}

/**
 * At this point the server is sending us the updates of the game from the last
 * tick we have sent up to the current simulation step. We have two things to
 * do:
 *   - transform the server's message into player_actions that we can use in
 *     the game.
 *   - remove our actions covered by the server's message.
 */
void bim::net::game_update_exchange::confirm_game_tick(
    const iscool::net::message& m)
{
  const std::optional<game_update_from_server> message =
      try_deserialize_message<game_update_from_server>(m);

  if (!message)
    {
      ic_log(iscool::log::nature::info(), "game_update_exchange",
             "Could not deserialize message.");
      return;
    }

  if (!validate_message(*message))
    return;

  store_server_frames(*message);
  remove_server_confirmed_actions();

  m_updated(m_server_update);
}

/// Filter inconsistent messages: corrupted, forged, lost in the network…
bool bim::net::game_update_exchange::validate_message(
    const game_update_from_server& message) const
{
  // The message should start from the last known server state.
  if (message.from_tick != m_current_update.from_tick)
    {
      // This happens quite frequently, so no log for it.
      return false;
    }

  if (message.actions.size() != m_player_count)
    return false;

  bim_assume(m_player_count >= 2);
  bim_assume(m_player_count <= bim::game::g_max_player_count);

  bool all_empty = true;

  for (std::size_t i = 0, n = m_player_count; i != n; ++i)
    {
      const std::size_t tick_count = message.actions[i].size();

      if (tick_count > 255)
        {
          ic_log(iscool::log::nature::info(), "game_update_exchange",
                 "Too many ticks from player {}: {}", i, tick_count);
          return false;
        }

      all_empty = all_empty && (tick_count == 0);
    }

  // No action, we are in sync with the server.
  return !all_empty;
}

void bim::net::game_update_exchange::store_server_frames(
    const game_update_from_server& message)
{
  bim_assume(m_player_count >= 2);
  bim_assume(m_player_count <= bim::game::g_max_player_count);

  m_server_update.from_tick = message.from_tick;

  for (int player_index = 0; player_index != m_player_count; ++player_index)
    {
      const std::vector<bim::game::player_action>& server_actions =
          message.actions[player_index];
      std::vector<bim::game::player_action>& local_actions =
          m_server_update.actions[player_index];

      local_actions.resize(server_actions.size());
      std::copy(server_actions.begin(), server_actions.end(),
                local_actions.begin());
    }
}

void bim::net::game_update_exchange::remove_server_confirmed_actions()
{
  std::size_t tick_count = m_server_update.actions[0].size();

  for (int player_index = 1; player_index != m_player_count; ++player_index)
    if (m_server_update.actions[player_index].size() > tick_count)
      tick_count = m_server_update.actions[player_index].size();

  m_current_update.from_tick += tick_count;
  m_current_update.actions.clear();

  bim_assume(tick_count <= m_action_queue.size());

  m_action_queue.erase(m_action_queue.begin(),
                       m_action_queue.begin() + tick_count);

  for (const bim::game::player_action& action : m_action_queue)
    if (!append_to_current_update(action))
      break;

  m_client_out_message = m_current_update.build_message();
}

void bim::net::game_update_exchange::dispatch_game_over(
    const iscool::net::message& m) const
{
  const std::optional<game_over> message =
      try_deserialize_message<game_over>(m);

  if (!message)
    {
      ic_log(iscool::log::nature::info(), "game_update_exchange",
             "Could not deserialize game over message.");
      return;
    }

  const std::uint8_t winner = message->get_winner_index();

  if (winner == bim::game::g_max_player_count)
    m_game_over(bim::game::contest_result::create_draw());
  else
    {
      if (winner >= m_player_count)
        {
          ic_log(iscool::log::nature::info(), "game_update_exchange",
                 "Winner is out of range.");
          return;
        }

      m_game_over(bim::game::contest_result::create_game_over(winner));
    }
}
