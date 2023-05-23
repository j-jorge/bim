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

#include <iscool/net/message_stream.h>

#include <optional>
#include <unordered_map>

namespace bm::server
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

  private:
    struct game;
    using game_map = std::unordered_map<iscool::net::channel_id, game>;

  private:
    iscool::net::message_stream m_message_stream;
    iscool::net::channel_id m_next_game_channel;
    game_map m_games;
  };
}
