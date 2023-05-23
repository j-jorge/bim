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
#pragma once

#include <bm/server/service/authentication_service.hpp>
#include <bm/server/service/matchmaking_service.hpp>

#include <iscool/net/socket_stream.h>

namespace bm::server
{
  class server
  {
  public:
    explicit server(unsigned short port);

  private:
    void dispatch(const iscool::net::endpoint& endpoint,
                  const iscool::net::message& message);

  private:
    iscool::net::socket_stream m_socket;
    bm::server::authentication_service m_authentication_service;
    bm::server::matchmaking_service m_matchmaking_service;
  };
}
