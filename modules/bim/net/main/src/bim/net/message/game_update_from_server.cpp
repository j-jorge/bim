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
  std::uint8_t size;

  reader >> from_tick >> player_count >> size;

  actions.resize(player_count);

  iscool::net::byte_array_bit_reader bits(reader);

  for (std::vector<bim::game::player_action>& v : actions)
    {
      v.resize(size);

      for (bim::game::player_action& action : v)
        read(action, bits);
    }
}

iscool::net::message bim::net::game_update_from_server::build_message() const
{
  iscool::net::byte_array content;

  const std::uint8_t player_count = actions.size();
  assert(player_count > 0);

  const std::uint8_t size = actions[0].size();

  content << from_tick << player_count << size;

  iscool::net::byte_array_bit_inserter bits(content);

  for (const std::vector<bim::game::player_action>& v : actions)
    {
      assert(v.size() == size);

      for (const bim::game::player_action& action : v)
        write(bits, action);
    }

  bits.flush();

  return iscool::net::message(get_type(), std::move(content));
}
