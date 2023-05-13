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

#include <bm/message/authentication.hpp>

#include <iscool/net/message_deserializer.impl.tpp>

bm::server::server::server(unsigned short port)
  : m_socket(port)
  , m_message_channel(m_socket, 0, 0, iscool::net::xor_key{})
{
  m_message_channel.connect_to_message(std::bind(
      &iscool::net::message_deserializer::interpret_received_message,
      &m_message_deserializer, std::placeholders::_1, std::placeholders::_2));

  m_message_deserializer.connect_signal<bm::message::authentication>(
      std::bind(&server::check_authentication, this, std::placeholders::_1,
                std::placeholders::_2));
}

void bm::server::server::check_authentication(
    const iscool::net::endpoint& endpoint,
    const bm::message::authentication& message)
{}
