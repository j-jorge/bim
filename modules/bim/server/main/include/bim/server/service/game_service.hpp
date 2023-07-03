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

#include <iscool/net/message_stream.hpp>

#include <optional>
#include <unordered_map>

namespace bim::net
{
  class game_update_from_client;
}

namespace bim::server
{
  struct game_info;

  class game_service
  {
  public:
    explicit game_service(iscool::net::socket_stream& socket);
    ~game_service();

    std::optional<game_info> find_game(iscool::net::channel_id channel) const;

    game_info new_game(std::uint8_t player_count,
                       const std::array<iscool::net::session_id, 4>& sessions);

    void process(const iscool::net::endpoint& endpoint,
                 const iscool::net::message& message);

  private:
    struct game;
    using game_map = std::unordered_map<iscool::net::channel_id, game>;

  private:
    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       iscool::net::channel_id channel, game& game);
    void push_update(const iscool::net::endpoint& endpoint,
                     iscool::net::session_id session,
                     iscool::net::channel_id channel,
                     const bim::net::game_update_from_client& message,
                     game& game);

    std::optional<std::size_t>
    validate_message(const bim::net::game_update_from_client& message,
                     std::size_t player_index, const game& game) const;
    void queue_actions(const bim::net::game_update_from_client& message,
                       std::size_t player_index, game& game);
    void send_actions(const iscool::net::endpoint& endpoint,
                      iscool::net::session_id session,
                      iscool::net::channel_id channel,
                      std::size_t player_index, game& game);

  private:
    iscool::net::message_stream m_message_stream;
    iscool::net::channel_id m_next_game_channel;
    game_map m_games;
  };
}
