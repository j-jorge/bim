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
#include <bm/net/exchange/authentication_exchange.hpp>

#include <bm/net/message/authentication.hpp>
#include <bm/net/message/authentication_ko.hpp>
#include <bm/net/message/authentication_ok.hpp>
#include <bm/net/message/protocol_version.hpp>

#include <iscool/net/message_deserializer.impl.tpp>
#include <iscool/random/rand.h>
#include <iscool/schedule/delayed_call.h>
#include <iscool/signals/implement_signal.h>

IMPLEMENT_SIGNAL(bm::net::authentication_exchange, authenticated,
                 m_authenticated);
IMPLEMENT_SIGNAL(bm::net::authentication_exchange, error, m_error);

bm::net::authentication_exchange::authentication_exchange(
    iscool::net::message_stream& stream)
  : m_message_channel(stream, 0, 0)
{
  m_deserializer.connect_signal<authentication_ok>(std::bind(
      &authentication_exchange::check_ok, this, std::placeholders::_2));
  m_deserializer.connect_signal<authentication_ko>(std::bind(
      &authentication_exchange::check_ko, this, std::placeholders::_2));
}

bm::net::authentication_exchange::~authentication_exchange() = default;

void bm::net::authentication_exchange::start()
{
  // TODO: random<client_token>()
  m_token = iscool::random::rand::get_default().random();
  m_client_message = authentication(protocol_version, m_token).build_message();

  m_channel_signal_connection = m_message_channel.connect_to_message(std::bind(
      &iscool::net::message_deserializer::interpret_received_message,
      &m_deserializer, std::placeholders::_1, std::placeholders::_2));

  tick();
}

void bm::net::authentication_exchange::stop()
{
  m_channel_signal_connection.disconnect();
  m_update_connection.disconnect();
}

void bm::net::authentication_exchange::tick()
{
  m_update_connection = iscool::schedule::delayed_call(
      std::bind(&authentication_exchange::tick, this),
      std::chrono::seconds(1));
  m_message_channel.send(m_client_message);
}

void bm::net::authentication_exchange::check_ok(
    const authentication_ok& message)
{
  if (message.get_request_token() == m_token)
    {
      stop();
      m_authenticated(message.get_session_id());
    }
}

void bm::net::authentication_exchange::check_ko(
    const authentication_ko& message)
{
  if (message.get_request_token() == m_token)
    {
      stop();
      m_error(message.get_error_code());
    }
}
