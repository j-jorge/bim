/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bim/net/exchange/game_update_exchange.hpp>

#include <bim/net/message/game_update_from_server.hpp>
#include <bim/net/message/ready.hpp>
#include <bim/net/message/start.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <bim/game/constant/max_player_count.hpp>

#include <bim/assume.hpp>

#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

namespace
{
  static constexpr int g_max_ticks_in_update_message = 64;
}

ic_implement_state_monitor(bim::net::game_update_exchange, m_monitor, idle,
                           ((idle)((start))) //
                           ((start)((play))) //
                           ((play)()));

IMPLEMENT_SIGNAL(bim::net::game_update_exchange, started, m_started);
IMPLEMENT_SIGNAL(bim::net::game_update_exchange, updated, m_updated);

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
  // If the message would become too large when adding the provided action,
  // then keep it for later. We need the server to confirm the previous actions
  // first.
  if (m_current_update.actions.size() >= g_max_ticks_in_update_message)
    return false;

  m_current_update.actions.push_back(action);
  return true;
}

void bim::net::game_update_exchange::send()
{
  m_message_channel.send(m_client_out_message);

  m_send_connection = iscool::schedule::delayed_call(
      std::bind(&game_update_exchange::send, this),
      std::chrono::milliseconds(100));
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

  const std::uint32_t tick_count = validate_message(*message);

  if (tick_count == 0)
    return;

  bim_assume(tick_count <= m_action_queue.size());

  store_server_frames(*message, tick_count);
  remove_server_confirmed_actions(tick_count);

  m_updated(m_server_update);
}

/// Filter inconsistent messages: corrupted, forged, lost in the network…
std::uint32_t bim::net::game_update_exchange::validate_message(
    const game_update_from_server& message) const
{
  // The message should start from the last known server state.
  if (message.from_tick != m_current_update.from_tick)
    {
      ic_log(iscool::log::nature::info(), "game_update_exchange",
             "Out of sync message, got %d, expected %d.", message.from_tick,
             m_current_update.from_tick);
      return 0;
    }

  if (message.actions.size() != m_player_count)
    return 0;

  bim_assume(m_player_count >= 2);
  bim_assume(m_player_count <= bim::game::g_max_player_count);

  const std::size_t tick_count = message.actions[0].size();

  // No action, we are in sync with the server.
  if (tick_count == 0)
    return 0;

  for (std::size_t i = 1, n = m_player_count; i != n; ++i)
    if (message.actions[i].size() != tick_count)
      {
        ic_log(iscool::log::nature::info(), "game_update_exchange",
               "Inconsistent tick count %d for player %d (expected %d).",
               message.actions[i].size(), i, tick_count);
        return 0;
      }

  // Suspiciously large message.
  if (tick_count > 255)
    {
      ic_log(iscool::log::nature::info(), "game_update_exchange",
             "Too many ticks: %d", tick_count);
      return 0;
    }

  return tick_count;
}

void bim::net::game_update_exchange::store_server_frames(
    const game_update_from_server& message, std::uint32_t tick_count)
{
  bim_assume(m_player_count >= 2);
  bim_assume(m_player_count <= bim::game::g_max_player_count);

  m_server_update.from_tick = message.from_tick;

  for (int player_index = 0; player_index != m_player_count; ++player_index)
    {
      m_server_update.actions[player_index].resize(tick_count);
      std::copy_n(message.actions[player_index].begin(), tick_count,
                  m_server_update.actions[player_index].begin());
    }
}

void bim::net::game_update_exchange::remove_server_confirmed_actions(
    std::uint32_t tick_count)
{
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
