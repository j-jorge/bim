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

#include <bim/game/component/player_action_kind_fwd.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  class game_update_from_client
  {
  public:
    static iscool::net::message_type get_type()
    {
      return message_type::game_update_from_client;
    }

    game_update_from_client();
    explicit game_update_from_client(
        const iscool::net::byte_array& raw_content);

    iscool::net::message build_message() const;

    std::size_t message_size() const;

  public:
    std::uint32_t from_tick;
    std::vector<std::uint8_t> action_count_at_tick;
    std::vector<bim::game::player_action_kind> actions;
  };
}
