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

#include <bim/net/message/message_type.hpp>

#include <bim/game/component/player_action.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
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

  public:
    /** Tick from which to play the following actions apply. */
    std::uint32_t from_tick;

    /** The actual actions, per player. */
    std::vector<std::vector<bim::game::player_action>> actions;
  };
}
