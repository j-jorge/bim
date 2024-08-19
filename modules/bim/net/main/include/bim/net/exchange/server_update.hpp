// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/max_player_count.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace bim::net
{
  struct server_update
  {
    std::uint32_t from_tick;
    std::array<std::vector<bim::game::player_action>,
               bim::game::g_max_player_count>
        actions;
  };
}
