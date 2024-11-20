// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/player_action.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>

#include <entt/entity/registry.hpp>

bim::game::player_action*
bim::game::find_player_action_by_index(entt::registry& registry,
                                       int player_index)
{
  for (auto&& [entity, player, action] :
       registry.view<player, player_action>().each())
    if (player.index == player_index)
      return &action;

  return nullptr;
}

void bim::game::collect_player_actions(std::span<player_action*> actions,
                                       entt::registry& registry)
{
  registry.view<bim::game::player, bim::game::player_action>().each(
      [actions](const bim::game::player& player,
                bim::game::player_action& action)
      {
        assert(player.index < actions.size());
        actions[player.index] = &action;
      });
}
