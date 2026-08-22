// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_result.hpp>

#include <bim/game/player_game_outcome.hpp>

#include <cassert>

enum class bim::game::contest_result::state : std::uint8_t
{
  still_running,
  won,
  over
};

bim::game::contest_result bim::game::contest_result::create_still_running()
{
  contest_result result;
  result.m_state = state::still_running;
  return result;
}

bim::game::contest_result bim::game::contest_result::create_victory(
    std::uint8_t winning_player,
    const per_player_array<player_game_outcome>& outcome)
{
  assert(winning_player < outcome.size());
  assert(outcome[winning_player] == player_game_outcome::victory);

  contest_result result;
  result.m_winning_player = winning_player;
  result.m_state = state::won;
  result.m_outcome = outcome;

  return result;
}

bim::game::contest_result bim::game::contest_result::create_game_over(
    const per_player_array<player_game_outcome>& outcome)
{
  contest_result result;
  result.m_state = state::over;
  result.m_outcome = outcome;

  return result;
}

bool bim::game::contest_result::still_running() const
{
  return m_state == state::still_running;
}

bool bim::game::contest_result::has_a_winner() const
{
  return m_state == state::won;
}

std::uint8_t bim::game::contest_result::winning_player() const
{
  assert(has_a_winner());
  return m_winning_player;
}

const bim::game::per_player_array<bim::game::player_game_outcome>&
bim::game::contest_result::outcome() const
{
  assert(!still_running());
  return m_outcome;
}
