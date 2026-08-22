// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/message_type.hpp>

#include <bim/game/per_player_array.hpp>
#include <bim/game/player_game_outcome_fwd.hpp>

#include <iscool/net/message/raw_message.hpp>

#include <cstdint>

namespace bim::net
{
  class game_over
  {
  public:
    static iscool::net::message_type get_type()
    {
      return message_type::game_over;
    }

    game_over(
        std::uint8_t winner_index,
        const bim::game::per_player_array<bim::game::player_game_outcome>&
            outcome,
        std::uint16_t coins_reward);
    game_over(const iscool::net::byte_array& raw_content);

    void build_message(iscool::net::message& message) const;

    std::uint8_t get_winner_index() const;
    const bim::game::per_player_array<bim::game::player_game_outcome>&
    get_outcome() const;
    std::uint16_t get_coins_reward() const;

  private:
    std::uint8_t m_winner_index;
    bim::game::per_player_array<bim::game::player_game_outcome> m_outcome;
    std::uint16_t m_coins_reward;
  };
}
