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
#include <bm/server/server.hpp>

#include <bm/net/message/authentication.hpp>
#include <bm/net/message/authentication_ko.hpp>
#include <bm/net/message/authentication_ok.hpp>
#include <bm/net/message/protocol_version.hpp>

#include <iscool/net/message_deserializer.impl.tpp>

bm::server::server::server(unsigned short port)
  : m_socket(port)
  , m_message_channel(m_socket, 0, 0, iscool::net::xor_key{})
{
  m_message_channel.connect_to_message(std::bind(
      &iscool::net::message_deserializer::interpret_received_message,
      &m_message_deserializer, std::placeholders::_1, std::placeholders::_2));

  m_message_deserializer.connect_signal<bm::net::authentication>(
      std::bind(&server::check_authentication, this, std::placeholders::_1,
                std::placeholders::_2));
}

void bm::server::server::check_authentication(
    const iscool::net::endpoint& endpoint,
    const bm::net::authentication& message)
{
  const bm::net::client_token token = message.get_request_token();

  if (message.get_protocol_version() != bm::net::protocol_version)
    {
      m_message_channel.send(
          endpoint,
          bm::net::authentication_ko(
              token, bm::net::authentication_error_code::bad_protocol)
              .build_message());
      return;
    }

  const iscool::net::session_id session = m_next_session_id;

  client_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_clients.emplace(token, session);

  if (inserted)
    ++m_next_session_id;

  m_sessions_time_to_live[session] = std::chrono::minutes(10);
  m_message_channel.send(
      endpoint, bm::net::authentication_ok(token, it->second).build_message());
}
