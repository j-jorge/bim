// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/matchmaking_service.hpp>

#include <bim/net/message/encounter_id.hpp>

#include <iscool/net/endpoint.hpp>
#include <iscool/net/message/session_id.hpp>

namespace iscool::net
{
  class socket_stream;
}

namespace bim::net
{
  class accept_random_game;
  class new_random_game_request;
}

namespace bim::server
{
  class game_service;

  struct config;

  class random_game_encounter_service
  {
  public:
    random_game_encounter_service(const config& config,
                                  iscool::net::socket_stream& socket,
                                  game_service& game_service);
    ~random_game_encounter_service();

    void process(const iscool::net::endpoint& endpoint,
                 iscool::net::session_id session,
                 const bim::net::new_random_game_request& request);
    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       const bim::net::accept_random_game& message);

  private:
    using session_to_encounter_map =
        boost::unordered_map<iscool::net::session_id, bim::net::encounter_id>;

  private:
    void clean_up();

  private:
    const game_service& m_game_service;
    matchmaking_service m_matchmaking_service;

    session_to_encounter_map m_session_to_encounter;
  };
}
