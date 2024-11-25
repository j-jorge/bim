// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/kick_event.hpp>

#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>

#include <entt/entity/registry.hpp>

void bim::game::kick_player(entt::registry& registry, int player_index)
{
  for (auto&& [entity, player] : registry.view<player>().each())
    if (player.index == player_index)
      registry.emplace<kicked>(entity);
}

std::array<std::size_t, bim::game::g_max_player_count>
bim::game::find_kick_event_tick(
    const std::array<std::vector<player_action>, g_max_player_count>& actions,
    std::size_t limit)
{
  std::array<std::size_t, g_max_player_count> result;

  for (std::size_t i = 0; i != result.size(); ++i)
    result[i] = std::min(actions[i].size(), limit);

  return result;
}
