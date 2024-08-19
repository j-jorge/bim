// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/matchmaking_service.hpp>
#include <bim/server/service/named_game_encounter_service.hpp>
#include <bim/server/service/random_game_encounter_service.hpp>

#include <iscool/net/message_stream.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <string>

namespace bim::net
{
  class accept_game;
  class new_game_request;
}

namespace bim::server
{
  class game_service;

  class lobby_service
  {
  public:
    lobby_service(iscool::net::socket_stream& socket,
                  game_service& game_service);
    ~lobby_service();

    void process(const iscool::net::endpoint& endpoint,
                 const iscool::net::message& message);

  private:
    void handle_new_game_request(const iscool::net::endpoint& endpoint,
                                 const iscool::net::message& m);
    void handle_new_random_game_request(const iscool::net::endpoint& endpoint,
                                        const iscool::net::message& m);
    void handle_accept_game(const iscool::net::endpoint& endpoint,
                            const iscool::net::message& m);

  private:
    matchmaking_service m_matchmaking_service;

    named_game_encounter_service m_named_game_encounter;
    random_game_encounter_service m_random_game_encounter;
  };
}
