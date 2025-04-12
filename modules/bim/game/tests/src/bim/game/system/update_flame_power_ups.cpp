// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flame_power_ups.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/constant/max_bomb_strength.hpp>
#include <bim/game/factory/flame_power_up.hpp>
#include <bim/game/factory/player.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_flame_power_ups, player_collision)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity power_up_entity =
      flame_power_up_factory(registry, arena, x, y);
  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{});

  const bim::game::player& player =
      registry.get<bim::game::player>(player_entity);

  const int bomb_strength = player.bomb_strength;

  bim::game::update_flame_power_ups(registry, arena);

  EXPECT_EQ(bomb_strength + 1, player.bomb_strength);
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(power_up_entity));
}

TEST(update_flame_power_ups, two_players_only_one_get_the_power_up)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  flame_power_up_factory(registry, arena, x, y);

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{}),
    bim::game::player_factory(registry, 1, x, y, bim::game::animation_id{})
  };

  bim::game::player* players[2] = {
    &registry.get<bim::game::player>(player_entity[0]),
    &registry.get<bim::game::player>(player_entity[1])
  };

  EXPECT_EQ(players[0]->bomb_strength, players[1]->bomb_strength);

  bim::game::update_flame_power_ups(registry, arena);

  EXPECT_EQ(1,
            std::abs(players[0]->bomb_strength - players[1]->bomb_strength));
}

TEST(update_flame_power_ups, max_capacity)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{});
  bim::game::player& player = registry.get<bim::game::player>(player_entity);
  player.bomb_strength = 0;

  for (int i = 0; i != bim::game::g_max_bomb_strength; ++i)
    {
      flame_power_up_factory(registry, arena, x, y);
      bim::game::update_flame_power_ups(registry, arena);
    }

  EXPECT_EQ(bim::game::g_max_bomb_strength, player.bomb_strength);

  // Keep increasing the bomb strength for player, he should consume the
  // power-ups but not increase in strength.
  for (int i = 0; i != bim::game::g_max_bomb_strength; ++i)
    {
      flame_power_up_factory(registry, arena, x, y);
      bim::game::update_flame_power_ups(registry, arena);

      EXPECT_TRUE(entt::null == arena.entity_at(x, y));
      EXPECT_EQ(bim::game::g_max_bomb_strength, player.bomb_strength);
    }
}
