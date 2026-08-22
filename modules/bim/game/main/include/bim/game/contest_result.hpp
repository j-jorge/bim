// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/per_player_array.hpp>
#include <bim/game/player_game_outcome_fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class contest_result
  {
  public:
    static contest_result create_still_running();
    static contest_result
    create_victory(std::uint8_t winning_player,
                   const per_player_array<player_game_outcome>& outcome);
    static contest_result
    create_game_over(const per_player_array<player_game_outcome>& outcome);

    bool still_running() const;
    bool has_a_winner() const;
    std::uint8_t winning_player() const;

    const per_player_array<player_game_outcome>& outcome() const;

  private:
    enum class state : std::uint8_t;

  private:
    std::uint8_t m_winning_player;
    state m_state;
    per_player_array<player_game_outcome> m_outcome;
  };
}
