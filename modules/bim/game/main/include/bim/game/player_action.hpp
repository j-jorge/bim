// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <span>

namespace bim::game
{
  struct player_action;

  player_action* find_player_action_by_index(entt::registry& registry,
                                             int player_index);
  void collect_player_actions(std::span<player_action*> actions,
                              entt::registry& registry);
}
