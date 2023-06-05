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

#include <bm/net/message/message_type.hpp>

#include <bm/game/component/player_action_kind_fwd.hpp>

#include <iscool/net/message/raw_message.h>

namespace bm::net
{
  class game_update_from_server
  {
  public:
    static iscool::net::message_type get_type()
    {
      return message_type::game_update_from_server;
    }

    game_update_from_server();
    explicit game_update_from_server(
        const iscool::net::byte_array& raw_content);

    iscool::net::message build_message() const;

    std::size_t message_size() const;

  public:
    /** Tick from which to play the following actions apply. */
    std::uint32_t first_tick;

    /** The number of actions for each tick and player. */
    std::vector<std::uint8_t> action_count;

    /**
     * The actual actions. They are packed per tick and in the order of the
     * players: (tick 0, player 0, actions…), (tick 0, player 1, actions…),
     * (tick 1, player 0, actions…), etc.
     */
    std::vector<bm::game::player_action_kind> actions;
  };
}
