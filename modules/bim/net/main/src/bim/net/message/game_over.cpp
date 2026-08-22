// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/game_over.hpp>

#include <bim/game/player_game_outcome.hpp>

#include <iscool/net/byte_array_serialization/byte_array_array_serialization.hpp>

#include <cassert>

bim::net::game_over::game_over(
    std::uint8_t winner_index,
    const bim::game::per_player_array<bim::game::player_game_outcome>& outcome,
    std::uint16_t coins_reward)
  : m_winner_index(winner_index)
  , m_outcome(outcome)
  , m_coins_reward(coins_reward)
{
  assert(winner_index <= bim::game::g_max_player_count);
}

bim::net::game_over::game_over(const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);
  reader >> m_winner_index >> m_outcome >> m_coins_reward;
}

void bim::net::game_over::build_message(iscool::net::message& message) const
{
  message.reset(get_type());
  message.get_content() << m_winner_index << m_outcome << m_coins_reward;

  if (m_winner_index > bim::game::g_max_player_count)
    throw std::runtime_error("");

  for (const bim::game::player_game_outcome o : m_outcome)
    {
      switch (o)
        {
        case bim::game::player_game_outcome::defeated:
        case bim::game::player_game_outcome::kicked:
        case bim::game::player_game_outcome::draw:
        case bim::game::player_game_outcome::victory:
          continue;
        }
      throw std::runtime_error("");
    }
}

std::uint8_t bim::net::game_over::get_winner_index() const
{
  return m_winner_index;
}

const bim::game::per_player_array<bim::game::player_game_outcome>&
bim::net::game_over::get_outcome() const
{
  return m_outcome;
}

std::uint16_t bim::net::game_over::get_coins_reward() const
{
  return m_coins_reward;
}
