#include <bim/net/message/launch_game.hpp>

#include <iscool/net/byte_array_bit_inserter.hpp>
#include <iscool/net/byte_array_bit_reader.hpp>

#include <cassert>

iscool::net::message_type bim::net::launch_game::get_type()
{
  return message_type::launch_game;
}

bim::net::launch_game::launch_game(
    client_token request_token, std::uint64_t seed,
    iscool::net::channel_id game_channel, std::uint32_t feature_mask,
    std::uint8_t player_count, std::uint8_t player_index,
    std::uint8_t brick_wall_probability, std::uint8_t arena_width,
    std::uint8_t arena_height)
  : m_request_token(request_token)
  , m_seed(seed)
  , m_game_channel(game_channel)
  , m_feature_mask(feature_mask)
  , m_player_count(player_count)
  , m_player_index(player_index)
  , m_brick_wall_probability(brick_wall_probability)
  , m_arena_width(arena_width)
  , m_arena_height(arena_height)
{
  assert(player_count >= 1);
  assert(player_count <= 4);
  assert(player_index <= 3);
}

bim::net::launch_game::launch_game(const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);
  reader >> m_request_token >> m_seed >> m_game_channel >> m_feature_mask;

  iscool::net::byte_array_bit_reader bits(reader);

  m_player_count = bits.get(2) + 1;
  m_player_index = bits.get(2);

  reader >> m_brick_wall_probability >> m_arena_width >> m_arena_height;
}

iscool::net::message bim::net::launch_game::build_message() const
{
  iscool::net::byte_array content;

  content << m_request_token << m_seed << m_game_channel << m_feature_mask;

  iscool::net::byte_array_bit_inserter bits(content);

  // There is at most four players and there is always at least one player,
  // thus we can store the number of players minus one on two bits.
  bits.append(m_player_count - 1, 2);

  // With a maximum four players the player index is in [0, 3], so we can use
  // only two bits.
  bits.append(m_player_index, 2);

  bits.flush();

  content << m_brick_wall_probability << m_arena_width << m_arena_height;

  return iscool::net::message(get_type(), content);
}

bim::net::client_token bim::net::launch_game::get_request_token() const
{
  return m_request_token;
}

std::uint64_t bim::net::launch_game::get_seed() const
{
  return m_seed;
}

iscool::net::channel_id bim::net::launch_game::get_game_channel() const
{
  return m_game_channel;
}

std::uint32_t bim::net::launch_game::get_feature_mask() const
{
  return m_feature_mask;
}

std::uint8_t bim::net::launch_game::get_player_count() const
{
  return m_player_count;
}

std::uint8_t bim::net::launch_game::get_player_index() const
{
  return m_player_index;
}

std::uint8_t bim::net::launch_game::get_brick_wall_probability() const
{
  return m_brick_wall_probability;
}

std::uint8_t bim::net::launch_game::get_arena_width() const
{
  return m_arena_width;
}

std::uint8_t bim::net::launch_game::get_arena_height() const
{
  return m_arena_height;
}
