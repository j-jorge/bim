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

#include <bm/net/message/client_token.hpp>
#include <bm/net/message/game_id.hpp>

#include <iscool/net/message_stream.h>

#include <string>
#include <unordered_map>

namespace bm::net
{
  class accept_game;
  class new_game_request;
}

namespace bm::server
{
  class matchmaking_service
  {
  public:
    explicit matchmaking_service(iscool::net::socket_stream& socket);
    ~matchmaking_service();

    void process(const iscool::net::endpoint& endpoint,
                 const iscool::net::message& message);

  private:
    struct game_info;
    using game_map = std::unordered_map<bm::net::game_id, game_info>;
    using name_to_game_id_map
        = std::unordered_map<std::string, bm::net::game_id>;
    using game_id_to_name_map
        = std::unordered_map<bm::net::game_id, std::string>;

  private:
    void create_or_update_game(const iscool::net::endpoint& endpoint,
                               iscool::net::session_id session,
                               const bm::net::new_game_request& request);
    void create_game(const iscool::net::endpoint& endpoint,
                     const iscool::net::session_id session,
                     const bm::net::new_game_request& request,
                     std::string name);
    void update_game(const iscool::net::endpoint& endpoint,
                     const iscool::net::session_id session,
                     const bm::net::new_game_request& request,
                     bm::net::game_id game_id, game_info& game);
    void send_game_on_hold(const iscool::net::endpoint& endpoint,
                           bm::net::client_token token,
                           iscool::net::session_id session,
                           bm::net::game_id game_id,
                           std::uint8_t player_count);

    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       const bm::net::accept_game& message);

    void remove_inactive_sessions(bm::net::game_id game_id, game_info& game);

    iscool::signals::connection schedule_clean_up(bm::net::game_id game_id);

    void clean_up(bm::net::game_id game_id);

  private:
    iscool::net::message_stream m_message_stream;
    name_to_game_id_map m_game_ids;
    game_id_to_name_map m_game_names;
    game_map m_games;
    bm::net::game_id m_next_game_id;
  };
}
