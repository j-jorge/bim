// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/game_update_from_server.hpp>

#include <bim/net/message/player_action_serialization.hpp>

#include <iscool/net/byte_array_bit_inserter.hpp>
#include <iscool/net/byte_array_bit_reader.hpp>

bim::net::game_update_from_server::game_update_from_server() = default;

bim::net::game_update_from_server::game_update_from_server(
    const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);
  std::uint8_t player_count;

  reader >> from_tick >> player_count;
  actions.resize(player_count);

  for (std::vector<bim::game::player_action>& v : actions)
    {
      std::uint8_t size;
      reader >> size;
      v.resize(size);
    }

  iscool::net::byte_array_bit_reader bits(reader);

  for (std::vector<bim::game::player_action>& v : actions)
    for (bim::game::player_action& action : v)
      read(action, bits);
}

iscool::net::message bim::net::game_update_from_server::build_message() const
{
  iscool::net::byte_array content;

  const std::uint8_t player_count = actions.size();
  assert(player_count > 0);

  content << from_tick << player_count;

  for (const std::vector<bim::game::player_action>& v : actions)
    {
      const std::uint8_t size = v.size();
      content << size;
    }

  iscool::net::byte_array_bit_inserter bits(content);

  for (const std::vector<bim::game::player_action>& v : actions)
    for (const bim::game::player_action& action : v)
      write(bits, action);

  bits.flush();

  return iscool::net::message(get_type(), std::move(content));
}
