// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_result.hpp>

#include <cassert>

enum class bim::game::contest_result::state
{
  still_running,
  draw,
  over
};

bim::game::contest_result bim::game::contest_result::create_draw()
{
  contest_result result;
  result.m_state = state::draw;
  return result;
}

bim::game::contest_result bim::game::contest_result::create_still_running()
{
  contest_result result;
  result.m_state = state::still_running;
  return result;
}

bim::game::contest_result
bim::game::contest_result::create_game_over(std::uint8_t winning_player)
{
  contest_result result;
  result.m_winning_player = winning_player;
  result.m_state = state::over;
  return result;
}

bool bim::game::contest_result::still_running() const
{
  return m_state == state::still_running;
}

bool bim::game::contest_result::has_a_winner() const
{
  return m_state == state::over;
}

std::uint8_t bim::game::contest_result::winning_player() const
{
  assert(has_a_winner());
  return m_winning_player;
}
