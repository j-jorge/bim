// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

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
  class new_game_request;
}

namespace bim::server
{
  class game_service;
  class matchmaking_service;

  class named_game_encounter_service
  {
  public:
    named_game_encounter_service(iscool::net::socket_stream& socket,
                                 const game_service& game_service,
                                 matchmaking_service& matchmaking_service);
    ~named_game_encounter_service();

    void process(const iscool::net::endpoint& endpoint,
                 iscool::net::session_id session,
                 const bim::net::new_game_request& request);

  private:
    struct encounter_info;
    using encounter_map =
        boost::unordered_map<bim::net::encounter_id, encounter_info>;
    using name_to_encounter_id_map =
        boost::unordered_map<std::string, bim::net::encounter_id>;

  private:
    void create_encounter(const iscool::net::endpoint& endpoint,
                          iscool::net::session_id session,
                          const bim::net::new_game_request& request,
                          std::string name);
    void update_encounter(const iscool::net::endpoint& endpoint,
                          iscool::net::session_id session,
                          const bim::net::new_game_request& request,
                          bim::net::encounter_id encounter_id);

    iscool::signals::connection
    schedule_clean_up(bim::net::encounter_id encounter_id);

    void clean_up(bim::net::encounter_id encounter_id);

  private:
    const game_service& m_game_service;
    matchmaking_service& m_matchmaking_service;

    name_to_encounter_id_map m_encounter_ids;
    encounter_map m_encounters;
  };
}
