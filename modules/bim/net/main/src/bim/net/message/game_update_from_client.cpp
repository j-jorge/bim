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
#include <bim/net/message/game_update_from_client.hpp>

#include <bim/net/message/player_action_serialization.hpp>

#include <iscool/net/byte_array_bit_inserter.hpp>
#include <iscool/net/byte_array_bit_reader.hpp>

bim::net::game_update_from_client::game_update_from_client() = default;

bim::net::game_update_from_client::game_update_from_client(
    const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);
  std::uint8_t size;

  reader >> from_tick >> size;
  actions.resize(size);

  iscool::net::byte_array_bit_reader bits(reader);

  for (bim::game::player_action& action : actions)
    read(action, bits);
}

iscool::net::message bim::net::game_update_from_client::build_message() const
{
  iscool::net::byte_array content;
  content << from_tick << (std::uint8_t)actions.size();

  iscool::net::byte_array_bit_inserter bits(content);

  for (const bim::game::player_action& action : actions)
    write(bits, action);

  bits.flush();

  return iscool::net::message(get_type(), content);
}
