// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/matchmaking_service.hpp>

#include <bim/net/message/encounter_id.hpp>

#include <iscool/net/endpoint.hpp>
#include <iscool/net/message/session_id.hpp>
#include <iscool/signals/connection.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <string>

namespace iscool::net
{
  class socket_stream;
}

namespace bim::net
{
  class accept_named_game;
  class new_named_game_request;
}

namespace bim::server
{
  class game_service;

  struct config;

  class named_game_encounter_service
  {
  public:
    named_game_encounter_service(const config& config,
                                 iscool::net::socket_stream& socket,
                                 game_service& game_service);
    ~named_game_encounter_service();

    void process(const iscool::net::endpoint& endpoint,
                 iscool::net::session_id session,
                 const bim::net::new_named_game_request& request);

    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       const bim::net::accept_named_game& message);

  private:
    struct encounter_info;
    using encounter_map =
        boost::unordered_map<bim::net::encounter_id, encounter_info>;
    using name_to_encounter_id_map =
        boost::unordered_map<std::string, bim::net::encounter_id>;

  private:
    void create_encounter(const iscool::net::endpoint& endpoint,
                          iscool::net::session_id session,
                          const bim::net::new_named_game_request& request,
                          std::string name);
    void update_encounter(const iscool::net::endpoint& endpoint,
                          iscool::net::session_id session,
                          const bim::net::new_named_game_request& request,
                          bim::net::encounter_id encounter_id);

    void clean_up();

  private:
    const game_service& m_game_service;
    matchmaking_service m_matchmaking_service;

    name_to_encounter_id_map m_encounter_ids;
    encounter_map m_encounters;
  };
}
