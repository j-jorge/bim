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
#include <bim/net/message/game_update_from_server.hpp>

#include <iscool/net/byte_array_serialization/byte_array_vector_serialization.hpp>

#include <cassert>
#include <limits>

bim::net::game_update_from_server::game_update_from_server() = default;

bim::net::game_update_from_server::game_update_from_server(
    const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);
  reader >> first_tick >> action_count >> actions;
}

iscool::net::message bim::net::game_update_from_server::build_message() const
{
  iscool::net::byte_array content;
  content << first_tick << action_count << actions;
  return iscool::net::message(get_type(), std::move(content));
}

std::size_t bim::net::game_update_from_server::message_size() const
{
  // For both player_index.size() and player_tick_count.size().
  static constexpr std::size_t size_of_vector_size = sizeof(std::size_t);

  return sizeof(first_tick) + size_of_vector_size
         + action_count.size() * sizeof(action_count[0]) + size_of_vector_size
         + actions.size() * sizeof(actions[0]);
}
