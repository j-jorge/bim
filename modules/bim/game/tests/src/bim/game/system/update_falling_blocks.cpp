// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_falling_blocks.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/factory/bomb.hpp>
#include <bim/game/factory/falling_block.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/update_bombs.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_update_falling_blocks, inserts_static_wall)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);

  constexpr std::chrono::milliseconds fall_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const bim::game::position_on_grid position = { 1, 0 };
  const entt::entity e =
      bim::game::falling_block_factory(registry, position, fall_delay);
  const bim::game::timer& t = registry.storage<bim::game::timer>().get(e);

  for (bool stop = false; !stop;)
    {
      bim::game::update_timers(registry, tick_duration);

      ASSERT_FALSE(arena.is_static_wall(position.x, position.y))
          << "timer.duration=" << t.duration;

      stop = (t.duration.count() == 0);
      bim::game::update_falling_blocks(registry, arena);
    }

  EXPECT_TRUE(arena.is_static_wall(position.x, position.y));
  ASSERT_TRUE(registry.valid(e));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(e));
}

TEST(bim_game_update_falling_blocks, triggers_the_bombs)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);

  constexpr std::chrono::milliseconds fall_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const bim::game::position_on_grid position = { 1, 0 };
  const entt::entity falling_block_entity =
      bim::game::falling_block_factory(registry, position, fall_delay);
  const bim::game::timer& falling_block_timer =
      registry.storage<bim::game::timer>().get(falling_block_entity);

  const entt::entity bomb_entity = bim::game::bomb_factory(
      registry, position.x, position.y, 10, 1, std::chrono::minutes(10));
  arena.put_entity(position.x, position.y, bomb_entity);
  const bim::game::timer& bomb_timer =
      registry.storage<bim::game::timer>().get(bomb_entity);

  while (falling_block_timer.duration.count() != 0)
    {
      EXPECT_FALSE(
          registry.storage<bim::game::dead>().contains(falling_block_entity));
      EXPECT_FALSE(registry.storage<bim::game::dead>().contains(bomb_entity));

      bim::game::update_timers(registry, tick_duration);
      bim::game::update_falling_blocks(registry, arena);
      bim::game::update_bombs(registry, arena);
    }

  EXPECT_GE(bomb_timer.duration.count(), 0);

  EXPECT_TRUE(
      registry.storage<bim::game::dead>().contains(falling_block_entity));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(bomb_entity));

  const entt::entity flame_entity =
      arena.entity_at(position.x + 1, position.y);
  ASSERT_TRUE(registry.valid(flame_entity));
  EXPECT_TRUE(registry.storage<bim::game::flame>().contains(flame_entity));
}

TEST(bim_game_update_falling_blocks, kills_the_players)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);

  constexpr std::chrono::milliseconds fall_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const bim::game::position_on_grid position = { 1, 0 };
  const entt::entity falling_block_entity =
      bim::game::falling_block_factory(registry, position, fall_delay);
  const bim::game::timer& falling_block_timer =
      registry.storage<bim::game::timer>().get(falling_block_entity);

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, 0, position.x, position.y),
    bim::game::player_factory(registry, 1, position.x, position.y)
  };

  while (falling_block_timer.duration.count() != 0)
    {
      EXPECT_FALSE(
          registry.storage<bim::game::dead>().contains(falling_block_entity));
      EXPECT_FALSE(
          registry.storage<bim::game::dead>().contains(player_entity[0]));
      EXPECT_FALSE(
          registry.storage<bim::game::dead>().contains(player_entity[1]));

      bim::game::update_timers(registry, tick_duration);
      bim::game::update_falling_blocks(registry, arena);
    }

  EXPECT_TRUE(
      registry.storage<bim::game::dead>().contains(falling_block_entity));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(player_entity[0]));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(player_entity[1]));
}
