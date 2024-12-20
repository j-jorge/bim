// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/named_game_encounter_service.hpp>
#include <bim/server/service/random_game_encounter_service.hpp>

#include <iscool/net/message_stream.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <string>

namespace bim::server
{
  class game_service;

  struct config;

  class lobby_service
  {
  public:
    lobby_service(const config& config, iscool::net::socket_stream& socket,
                  game_service& game_service);
    ~lobby_service();

    void process(const iscool::net::endpoint& endpoint,
                 const iscool::net::message& message);

  private:
    void handle_new_named_game_request(const iscool::net::endpoint& endpoint,
                                       const iscool::net::message& m);
    void handle_new_random_game_request(const iscool::net::endpoint& endpoint,
                                        const iscool::net::message& m);
    void handle_accept_named_game(const iscool::net::endpoint& endpoint,
                                  const iscool::net::message& m);
    void handle_accept_random_game(const iscool::net::endpoint& endpoint,
                                   const iscool::net::message& m);

  private:
    named_game_encounter_service m_named_game_encounter;
    random_game_encounter_service m_random_game_encounter;
  };
}
