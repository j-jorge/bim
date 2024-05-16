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

#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/message/accept_game.hpp>
#include <bim/net/message/game_on_hold.hpp>
#include <bim/net/message/launch_game.hpp>
#include <bim/net/message/new_game_request.hpp>
#include <bim/net/message/new_random_game_request.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/info.hpp>
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
    const iscool::net::message_stream& stream)
  : m_message_channel(stream)
{}

bim::net::new_game_exchange::~new_game_exchange() = default;

void bim::net::new_game_exchange::start(iscool::net::session_id session,
                                        const game_name& name)
{
  ic_causeless_log(iscool::log::nature::info(), "new_game_exchange",
                   "Requesting game '%s' in session %d.",
                   std::string_view((const char*)name.data(), name.size()),
                   session);

  internal_start(session);
  m_client_message = new_game_request(m_token, name).build_message();
  tick();
}

void bim::net::new_game_exchange::start(iscool::net::session_id session)
{
  ic_causeless_log(iscool::log::nature::info(), "new_game_exchange",
                   "Requesting random game in session %d.", session);

  internal_start(session);
  m_client_message = new_random_game_request(m_token).build_message();
  tick();
}

void bim::net::new_game_exchange::accept()
{
  assert(!!m_encounter_id);

  ic_causeless_log(iscool::log::nature::info(), "new_game_exchange",
                   "Accepting encounter %d.", *m_encounter_id);

  assert(m_update_connection.connected());

  m_monitor->set_accept_state();

  m_client_message = accept_game(m_token, *m_encounter_id).build_message();
}

void bim::net::new_game_exchange::stop()
{
  if (m_monitor->is_stop_state() || m_monitor->is_idle_state())
    return;

  m_monitor->set_stop_state();

  m_encounter_id = std::nullopt;
  m_channel_signal_connection.disconnect();
  m_update_connection.disconnect();

  m_monitor->set_idle_state();
}

void bim::net::new_game_exchange::internal_start(
    iscool::net::session_id session)
{
  assert(!m_encounter_id);

  m_monitor->set_start_state();

  m_message_channel.rebind(session, 0);

  // TODO: random<client_token>()
  m_token = iscool::random::rand::get_default().random();

  m_channel_signal_connection = m_message_channel.connect_to_message(
      std::bind(&new_game_exchange::interpret_received_message, this,
                std::placeholders::_2));

  tick();
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
      check_on_hold(message);
      break;
    case message_type::launch_game:
      check_launch_game(message);
      break;
    }
}

void bim::net::new_game_exchange::check_on_hold(const iscool::net::message& m)
{
  if (!m_monitor->is_start_state())
    return;

  const std::optional<game_on_hold> message =
      try_deserialize_message<game_on_hold>(m);

  if (!message)
    return;

  if (message->get_request_token() != m_token)
    return;

  if (!m_encounter_id)
    ic_causeless_log(iscool::log::nature::info(), "new_game_exchange",
                     "Got game proposal %d.", message->get_encounter_id());

  m_encounter_id = message->get_encounter_id();

  m_game_proposal(message->get_player_count());
}

void bim::net::new_game_exchange::check_launch_game(
    const iscool::net::message& m)
{
  if (!m_monitor->is_accept_state())
    return;

  const std::optional<launch_game> message =
      try_deserialize_message<launch_game>(m);

  if (!message)
    return;

  if (message->get_request_token() != m_token)
    return;

  ic_causeless_log(iscool::log::nature::info(), "new_game_exchange",
                   "Launch game %d.", *m_encounter_id);

  stop();
  m_launch_game(
      game_launch_event{ .seed = message->get_seed(),
                         .channel = message->get_game_channel(),
                         .player_count = message->get_player_count(),
                         .player_index = message->get_player_index() });
}
