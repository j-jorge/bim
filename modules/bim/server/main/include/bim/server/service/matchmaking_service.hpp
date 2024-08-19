// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>

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

  class matchmaking_service
  {
  public:
    matchmaking_service(iscool::net::socket_stream& socket,
                        game_service& game_service);
    ~matchmaking_service();

    bim::net::encounter_id new_encounter(const iscool::net::endpoint& endpoint,
                                         iscool::net::session_id session,
                                         bim::net::client_token request_token);
    bool refresh_encounter(bim::net::encounter_id encounter_id,
                           const iscool::net::endpoint& endpoint,
                           iscool::net::session_id session,
                           bim::net::client_token request_token);
    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       const bim::net::accept_game& message);

  private:
    struct encounter_info;
    using encounter_map =
        boost::unordered_map<bim::net::encounter_id, encounter_info>;

  private:
    void send_game_on_hold(const iscool::net::endpoint& endpoint,
                           bim::net::client_token token,
                           iscool::net::session_id session,
                           bim::net::encounter_id encounter_id,
                           std::uint8_t player_count);

    void remove_inactive_sessions(bim::net::encounter_id encounter_id,
                                  encounter_info& encounter);

    iscool::signals::connection
    schedule_clean_up(bim::net::encounter_id encounter_id);

    void clean_up(bim::net::encounter_id encounter_id);

  private:
    iscool::net::message_stream m_message_stream;
    game_service& m_game_service;

    encounter_map m_encounters;
    bim::net::encounter_id m_next_encounter_id;
  };
}
