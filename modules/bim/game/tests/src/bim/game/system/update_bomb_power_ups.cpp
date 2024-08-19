// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_bomb_power_ups.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/constant/max_bomb_count_per_player.hpp>
#include <bim/game/factory/bomb_power_up.hpp>
#include <bim/game/factory/player.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_bomb_power_ups, increment_player_capacity_and_available)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity power_up_entity =
      bomb_power_up_factory(registry, arena, x, y);
  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y);

  bim::game::player& player = registry.get<bim::game::player>(player_entity);

  const int bomb_capacity = player.bomb_capacity;
  player.bomb_available = 0;
  const int bomb_available = player.bomb_available;

  bim::game::update_bomb_power_ups(registry, arena);

  EXPECT_EQ(bomb_capacity + 1, player.bomb_capacity);
  EXPECT_EQ(bomb_available + 1, player.bomb_available);
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(power_up_entity));
}

TEST(update_bomb_power_ups, two_players_only_one_get_the_power_up)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  bomb_power_up_factory(registry, arena, x, y);

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, 0, x, y),
    bim::game::player_factory(registry, 1, x, y)
  };

  bim::game::player* players[2] = {
    &registry.get<bim::game::player>(player_entity[0]),
    &registry.get<bim::game::player>(player_entity[1])
  };

  EXPECT_EQ(players[0]->bomb_capacity, players[1]->bomb_capacity);
  EXPECT_EQ(players[0]->bomb_available, players[1]->bomb_available);

  bim::game::update_bomb_power_ups(registry, arena);

  EXPECT_EQ(1,
            std::abs(players[0]->bomb_capacity - players[1]->bomb_capacity));
  EXPECT_EQ(1,
            std::abs(players[0]->bomb_available - players[1]->bomb_available));
}

TEST(update_bomb_power_ups, max_capacity)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y);
  bim::game::player& player = registry.get<bim::game::player>(player_entity);
  player.bomb_capacity = 0;

  for (int i = 0; i != bim::game::g_max_bomb_count_per_player; ++i)
    {
      bomb_power_up_factory(registry, arena, x, y);
      bim::game::update_bomb_power_ups(registry, arena);
    }

  EXPECT_EQ(bim::game::g_max_bomb_count_per_player, player.bomb_capacity);

  // Give more bombs to the player, he should take them but not have them.
  for (int i = 0; i != bim::game::g_max_bomb_count_per_player; ++i)
    {
      bomb_power_up_factory(registry, arena, x, y);
      bim::game::update_bomb_power_ups(registry, arena);

      EXPECT_TRUE(entt::null == arena.entity_at(x, y));
      EXPECT_EQ(bim::game::g_max_bomb_count_per_player, player.bomb_capacity);
    }
}
