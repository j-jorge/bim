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
#include <bim/net/exchange/authentication_exchange.hpp>

#include <bim/net/message/authentication.hpp>
#include <bim/net/message/authentication_ko.hpp>
#include <bim/net/message/authentication_ok.hpp>
#include <bim/net/message/protocol_version.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/random/rand.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::net::authentication_exchange, authenticated,
                 m_authenticated);
IMPLEMENT_SIGNAL(bim::net::authentication_exchange, error, m_error);

bim::net::authentication_exchange::authentication_exchange(
    iscool::net::message_stream& stream)
  : m_message_channel(stream, 0, 0)
{}

bim::net::authentication_exchange::~authentication_exchange() = default;

void bim::net::authentication_exchange::start()
{
  // TODO: random<client_token>()
  m_token = iscool::random::rand::get_default().random();
  m_client_message = authentication(protocol_version, m_token).build_message();

  m_channel_signal_connection = m_message_channel.connect_to_message(
      std::bind(&authentication_exchange::interpret_received_message, this,
                std::placeholders::_2));

  tick();
}

void bim::net::authentication_exchange::interpret_received_message(
    const iscool::net::message& message)
{
  switch (message.get_type())
    {
    case message_type::authentication_ok:
      check_ok(message);
      break;
    case message_type::authentication_ko:
      check_ko(message);
      break;
    }
}

void bim::net::authentication_exchange::stop()
{
  m_channel_signal_connection.disconnect();
  m_update_connection.disconnect();
}

void bim::net::authentication_exchange::tick()
{
  m_update_connection = iscool::schedule::delayed_call(
      std::bind(&authentication_exchange::tick, this),
      std::chrono::seconds(1));
  m_message_channel.send(m_client_message);
}

void bim::net::authentication_exchange::check_ok(const iscool::net::message& m)
{
  const std::optional<authentication_ok> message =
      try_deserialize_message<authentication_ok>(m);

  if (!message)
    return;

  if (message->get_request_token() == m_token)
    {
      stop();
      ic_causeless_log(iscool::log::nature::info(), "authentication_exchange",
                       "Authentication OK, session=%d",
                       message->get_session_id());
      m_authenticated(message->get_session_id());
    }
}

void bim::net::authentication_exchange::check_ko(const iscool::net::message& m)
{
  const std::optional<authentication_ko> message =
      try_deserialize_message<authentication_ko>(m);

  if (!message)
    return;

  if (message->get_request_token() == m_token)
    {
      stop();
      ic_causeless_log(iscool::log::nature::error(), "authentication_exchange",
                       "Authentication KO, error_code=%d",
                       (int)message->get_error_code());
      m_error(message->get_error_code());
    }
}
