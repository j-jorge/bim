// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  class contest_result
  {
  public:
    static contest_result create_draw();
    static contest_result create_still_running();
    static contest_result create_game_over(std::uint8_t winning_player);

    bool still_running() const;
    bool has_a_winner() const;
    std::uint8_t winning_player() const;

  private:
    enum class state;

  private:
    std::uint8_t m_winning_player;
    state m_state;
  };
}
