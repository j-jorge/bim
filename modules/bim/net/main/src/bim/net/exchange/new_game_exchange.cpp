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
#include <bim/net/exchange/new_game_exchange.hpp>

#include <bim/net/message/accept_game.hpp>
#include <bim/net/message/game_on_hold.hpp>
#include <bim/net/message/launch_game.hpp>
#include <bim/net/message/new_game_request.hpp>

#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/random/rand.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

ic_implement_state_monitor(bim::net::new_game_exchange, m_monitor, idle,
                           ((idle)((start)))         //
                           ((start)((accept)(stop))) //
                           ((accept)((stop)))        //
                           ((stop)((idle))));

IMPLEMENT_SIGNAL(bim::net::new_game_exchange, game_proposal, m_game_proposal);
IMPLEMENT_SIGNAL(bim::net::new_game_exchange, launch_game, m_launch_game);

bim::net::new_game_exchange::new_game_exchange(
    const iscool::net::message_stream& stream, iscool::net::session_id session)
  : m_message_channel(stream, session, 0)
{}

bim::net::new_game_exchange::~new_game_exchange() = default;

void bim::net::new_game_exchange::start(const game_name& name)
{
  assert(!m_encounter_id);

  m_monitor->set_start_state();

  // TODO: random<client_token>()
  m_token = iscool::random::rand::get_default().random();
  m_client_message = new_game_request(m_token, name).build_message();

  m_channel_signal_connection = m_message_channel.connect_to_message(
      std::bind(&new_game_exchange::interpret_received_message, this,
                std::placeholders::_2));

  tick();
}

void bim::net::new_game_exchange::accept()
{
  assert(!!m_encounter_id);
  assert(m_update_connection.connected());

  m_monitor->set_accept_state();

  m_client_message = accept_game(m_token, *m_encounter_id).build_message();
}

void bim::net::new_game_exchange::stop()
{
  m_monitor->set_stop_state();

  m_encounter_id = std::nullopt;
  m_channel_signal_connection.disconnect();
  m_update_connection.disconnect();

  m_monitor->set_idle_state();
}

void bim::net::new_game_exchange::tick()
{
  assert(!m_monitor->is_idle_state());

  m_update_connection = iscool::schedule::delayed_call(
      std::bind(&new_game_exchange::tick, this), std::chrono::seconds(1));
  m_message_channel.send(m_client_message);
}

void bim::net::new_game_exchange::interpret_received_message(
    const iscool::net::message& message)
{
  switch (message.get_type())
    {
    case message_type::game_on_hold:
      {
        if (!m_monitor->is_start_state())
          return;

        check_on_hold(game_on_hold(message.get_content()));
        break;
      }
    case message_type::launch_game:
      {
        if (!m_monitor->is_accept_state())
          return;

        check_launch_game(launch_game(message.get_content()));
        break;
      }
    }
}

void bim::net::new_game_exchange::check_on_hold(const game_on_hold& message)
{
  if (message.get_request_token() != m_token)
    return;

  if (!m_monitor->is_start_state())
    return;

  m_encounter_id = message.get_encounter_id();
  m_game_proposal(message.get_player_count());
}

void bim::net::new_game_exchange::check_launch_game(const launch_game& message)
{
  assert(m_monitor->is_accept_state());

  if (message.get_request_token() != m_token)
    return;

  stop();
  m_launch_game(message.get_game_channel(), message.get_player_count(),
                message.get_player_index());
}
