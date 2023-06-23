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
#include <bim/server/server.hpp>

#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/net/message/message.hpp>

bim::server::server::server(unsigned short port)
  : m_socket(port)
  , m_authentication_service(m_socket)
  , m_game_service(m_socket)
  , m_matchmaking_service(m_socket, m_game_service)
{
  ic_causeless_log(iscool::log::nature::info(), "server",
                   "Server is up on port %d.", port);

  m_authentication_service.connect_to_message(std::bind(
      &server::dispatch, this, std::placeholders::_1, std::placeholders::_2));
}

void bim::server::server::dispatch(const iscool::net::endpoint& endpoint,
                                   const iscool::net::message& message)
{
  // The message comes from an active session, we can process it.

  if (message.get_channel_id() == 0)
    m_matchmaking_service.process(endpoint, message);
  else
    m_game_service.process(endpoint, message);
}
