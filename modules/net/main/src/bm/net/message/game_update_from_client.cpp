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
#include <bm/net/message/game_update_from_client.hpp>

#include <iscool/net/byte_array_serialization/byte_array_vector_serialization.h>

bm::net::game_update_from_client::game_update_from_client() = default;

bm::net::game_update_from_client::game_update_from_client(
    const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);

  reader >> from_tick >> action_count_at_tick >> actions;
}

iscool::net::message bm::net::game_update_from_client::build_message() const
{
  iscool::net::byte_array content;
  content << from_tick << action_count_at_tick << actions;

  return iscool::net::message(get_type(), content);
}

std::size_t bm::net::game_update_from_client::message_size() const
{
  // For both action_count_at_tick.size() and actions.size().
  static constexpr std::size_t size_of_vector_size = sizeof(std::uint32_t);

  return sizeof(from_tick) + size_of_vector_size
         + action_count_at_tick.size() * sizeof(action_count_at_tick[0])
         + size_of_vector_size + actions.size() * sizeof(actions[0]);
}
