// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/constant/max_player_count.hpp>

#include <entt/entity/fwd.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace bim::game
{
  class player_action;

  void kick_player(entt::registry& registry, int player_index);

  std::array<std::size_t, g_max_player_count>
  find_kick_event_tick(const std::array<std::vector<player_action>,
                                        g_max_player_count>& actions,
                       std::size_t limit);
}
