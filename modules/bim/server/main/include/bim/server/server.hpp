// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/authentication_service.hpp>
#include <bim/server/service/game_service.hpp>
#include <bim/server/service/lobby_service.hpp>
#include <bim/server/service/matchmaking_service.hpp>

#include <iscool/net/socket_stream.hpp>

namespace bim::server
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
    authentication_service m_authentication_service;
    game_service m_game_service;
    lobby_service m_lobby_service;
  };
}
