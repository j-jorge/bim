// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/player_action.hpp>

#include <bim/game/component/player_action.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/player.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_player_action, find_player_action_by_index)
{
  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);

  const entt::entity player_0 = bim::game::player_factory(
      registry, entity_map, 0, 1, 0, bim::game::animation_id{});
  const entt::entity player_1 = bim::game::player_factory(
      registry, entity_map, 1, 1, 1, bim::game::animation_id{});

  EXPECT_EQ(&registry.storage<bim::game::player_action>().get(player_0),
            bim::game::find_player_action_by_index(registry, 0));

  EXPECT_EQ(&registry.storage<bim::game::player_action>().get(player_1),
            bim::game::find_player_action_by_index(registry, 1));

  EXPECT_EQ(nullptr, bim::game::find_player_action_by_index(registry, 2));
}
