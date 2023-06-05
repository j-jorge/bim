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
#include <bm/net/exchange/game_update_exchange.hpp>

#include <bm/net/message/game_update_from_server.hpp>
#include <bm/net/message/ready.hpp>
#include <bm/net/message/start.hpp>

#include <bm/game/assume.hpp>

#include <iscool/monitoring/implement_state_monitor.h>
#include <iscool/net/message_channel.h>
#include <iscool/schedule/delayed_call.h>
#include <iscool/signals/implement_signal.h>

namespace
{
  static constexpr int g_max_update_message_size_in_bytes = 480;
}

ic_implement_state_monitor(bm::net::game_update_exchange, m_monitor, idle,
                           ((idle)((start))) //
                           ((start)((play))) //
                           ((play)()));

IMPLEMENT_SIGNAL(bm::net::game_update_exchange, started, m_started);
IMPLEMENT_SIGNAL(bm::net::game_update_exchange, updated, m_updated);

bm::net::game_update_exchange::game_update_exchange(
    iscool::net::message_channel& channel, std::uint8_t player_count)
  : m_message_channel(channel)
  , m_player_count(player_count)
{
  m_channel_signal_connection = m_message_channel.connect_to_message(std::bind(
      &game_update_exchange::deserialize, this, std::placeholders::_2));
}

bm::net::game_update_exchange::~game_update_exchange() = default;

void bm::net::game_update_exchange::start()
{
  m_monitor->set_start_state();

  m_client_out_message = bm::net::ready().build_message();
  send();
}

void bm::net::game_update_exchange::push(const bm::game::player_action& action)
{
  m_action_queue.emplace_back(action);

  if (append_to_current_update(action))
    m_client_out_message = m_current_update.build_message();

  if (!m_send_connection.connected())
    send();
}

void bm::net::game_update_exchange::deserialize(
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
        confirm_game_tick(game_update_from_server(message.get_content()));
      break;
    }
}

void bm::net::game_update_exchange::dispatch_start()
{
  m_monitor->set_play_state();

  m_current_update.from_tick = 0;

  m_current_update.action_count_at_tick.clear();
  m_current_update.action_count_at_tick.reserve(64);

  m_current_update.actions.clear();
  m_current_update.actions.reserve(64);

  m_send_connection.disconnect();

  m_started();
}

bool bm::net::game_update_exchange::append_to_current_update(
    const bm::game::player_action& action)
{
  // If the message would become too large when adding the provided action,
  // then keep it for later. We need the server to confirm the previous actions
  // first.
  if (m_current_update.message_size() >= g_max_update_message_size_in_bytes)
    return false;

  m_current_update.action_count_at_tick.emplace_back(action.queue_size);
  m_current_update.actions.insert(m_current_update.actions.end(), action.queue,
                                  action.queue + action.queue_size);

  return true;
}

void bm::net::game_update_exchange::send()
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
void bm::net::game_update_exchange::confirm_game_tick(
    const game_update_from_server& message)
{
  const std::uint32_t tick_count = validate_message(message);

  if (tick_count == 0)
    return;

  bm_assume(tick_count <= m_action_queue.size());

  store_server_frames(message, tick_count);
  remove_server_confirmed_actions(tick_count);

  m_updated(m_server_update);
}

/// Filter inconsistent messages: corrupted, forged, lost in the network…
std::uint32_t bm::net::game_update_exchange::validate_message(
    const game_update_from_server& message) const
{
  // The message should start from the last known server state.
  if (message.first_tick != m_current_update.from_tick)
    return 0;

  // No action, we are in sync with the server.
  if (message.action_count.empty())
    return 0;

  bm_assume(m_player_count >= 2);
  bm_assume(m_player_count <= 4);

  if (message.action_count.size() % m_player_count != 0)
    return 0;

  const std::size_t tick_count = message.action_count.size() / m_player_count;

  // Suspiciously large message.
  if (tick_count > 255)
    return 0;

  std::size_t action_count = 0;

  for (std::uint8_t c : message.action_count)
    {
      if (action_count > bm::game::player_action::queue_capacity)
        return 0;

      action_count += c;
    }

  if (action_count != message.actions.size())
    return 0;

  return tick_count;
}

void bm::net::game_update_exchange::store_server_frames(
    const game_update_from_server& message, std::uint32_t tick_count)
{
  bm_assume(m_player_count >= 2);
  bm_assume(m_player_count <= 4);

  m_server_update.from_tick = message.first_tick;
  m_server_update.actions.resize(tick_count);

  for (std::size_t action_count_index = 0, n = message.action_count.size(),
                   action_index = 0, tick = 0;
       action_count_index != n;)
    {
      std::array<bm::game::player_action, 4>& frame_actions
          = m_server_update.actions[tick];
      ++tick;

      for (std::uint8_t player_index = 0; player_index != m_player_count;
           ++player_index)
        {
          const std::uint8_t action_count
              = message.action_count[action_count_index];
          ++action_count_index;

          frame_actions[player_index].queue_size = action_count;
          std::copy_n(message.actions.begin() + action_index, action_count,
                      frame_actions[player_index].queue);
          action_index += action_count;
        }
    }
}

void bm::net::game_update_exchange::remove_server_confirmed_actions(
    std::uint32_t tick_count)
{
  m_current_update.from_tick += tick_count;
  m_current_update.action_count_at_tick.clear();
  m_current_update.actions.clear();

  bm_assume(tick_count <= m_action_queue.size());

  for (std::size_t i = tick_count, n = m_action_queue.size(); i != n; ++i)
    if (!append_to_current_update(m_action_queue[i]))
      break;

  m_action_queue.erase(m_action_queue.begin(),
                       m_action_queue.begin() + tick_count);

  m_client_out_message = m_current_update.build_message();
}
