// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_falling_blocks.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/component/wall.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/factory/bomb.hpp>
#include <bim/game/factory/falling_block.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/update_bombs.hpp>
#include <bim/game/system/update_players.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_update_falling_blocks, inserts_wall)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);

  constexpr std::chrono::milliseconds fall_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const bim::game::position_on_grid position = { 1, 0 };
  const entt::entity e =
      bim::game::falling_block_factory(registry, position, fall_delay);
  const bim::game::timer& t = registry.storage<bim::game::timer>().get(e);

  for (bool stop = false; !stop;)
    {
      bim::game::update_timers(registry, tick_duration);

      ASSERT_TRUE(registry.storage<bim::game::wall>().empty())
          << "timer.duration=" << t.duration;

      stop = (t.duration.count() == 0);
      bim::game::update_falling_blocks(context, registry, entity_map);
    }

  ASSERT_EQ(1, entity_map.entities_at(position.x, position.y).size());

  const entt::entity wall_entity =
      entity_map.entities_at(position.x, position.y)[0];

  EXPECT_TRUE(registry.storage<bim::game::wall>().contains(wall_entity));
  ASSERT_TRUE(registry.valid(e));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(e));
}

TEST(bim_game_update_falling_blocks, triggers_the_bombs)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(3, 3);
  bim::game::entity_world_map entity_map(arena.width(), arena.height());

  constexpr std::chrono::milliseconds fall_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const bim::game::position_on_grid position = { 1, 0 };
  const entt::entity falling_block_entity =
      bim::game::falling_block_factory(registry, position, fall_delay);
  const bim::game::timer& falling_block_timer =
      registry.storage<bim::game::timer>().get(falling_block_entity);

  const entt::entity bomb_entity =
      bim::game::bomb_factory(registry, entity_map, position.x, position.y, 10,
                              1, std::chrono::minutes(10));
  const bim::game::timer& bomb_timer =
      registry.storage<bim::game::timer>().get(bomb_entity);

  while (falling_block_timer.duration.count() != 0)
    {
      EXPECT_FALSE(
          registry.storage<bim::game::dead>().contains(falling_block_entity));
      EXPECT_FALSE(registry.storage<bim::game::dead>().contains(bomb_entity));

      bim::game::update_timers(registry, tick_duration);
      bim::game::update_falling_blocks(context, registry, entity_map);
      bim::game::trigger_crushed_timers(registry);
      bim::game::update_bombs(context, registry, arena, entity_map);
    }

  EXPECT_GE(bomb_timer.duration.count(), 0);

  EXPECT_TRUE(
      registry.storage<bim::game::dead>().contains(falling_block_entity));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(bomb_entity));

  bool flame_left = false;
  bool flame_right = false;
  bool flame_down = false;

  registry.view<bim::game::flame, bim::game::position_on_grid>().each(
      [&](const bim::game::flame&, const bim::game::position_on_grid& p)
      {
        if ((p.x == position.x + 1) && (p.y == position.y))
          {
            EXPECT_FALSE(flame_right);
            flame_right = true;
          }
        else if ((p.x == position.x - 1) && (p.y == position.y))
          {
            EXPECT_FALSE(flame_left);
            flame_left = true;
          }
        else if ((p.x == position.x) && (p.y == position.y + 1))
          {
            EXPECT_FALSE(flame_down);
            flame_down = true;
          }
      });

  EXPECT_TRUE(flame_left);
  EXPECT_TRUE(flame_right);
  EXPECT_TRUE(flame_down);
}

TEST(bim_game_update_falling_blocks, kills_the_players)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);

  constexpr std::chrono::milliseconds fall_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const bim::game::position_on_grid position = { 1, 0 };
  const entt::entity falling_block_entity =
      bim::game::falling_block_factory(registry, position, fall_delay);
  const bim::game::timer& falling_block_timer =
      registry.storage<bim::game::timer>().get(falling_block_entity);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, entity_map, 0, position.x, position.y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 1, position.x, position.y,
                              player_animations.idle_down)
  };

  const bim::game::animation_state* const player_state[2] = {
    &registry.storage<bim::game::animation_state>().get(player_entity[0]),
    &registry.storage<bim::game::animation_state>().get(player_entity[1])
  };

  while (falling_block_timer.duration.count() != 0)
    {
      EXPECT_FALSE(
          registry.storage<bim::game::dead>().contains(falling_block_entity));
      EXPECT_TRUE(player_animations.is_alive(player_state[0]->model));
      EXPECT_TRUE(player_animations.is_alive(player_state[1]->model));

      bim::game::update_timers(registry, tick_duration);
      bim::game::update_falling_blocks(context, registry, entity_map);
      bim::game::trigger_crushed_timers(registry);
      bim::game::update_players(context, registry);
    }

  EXPECT_TRUE(
      registry.storage<bim::game::dead>().contains(falling_block_entity));
  EXPECT_FALSE(registry.storage<bim::game::dead>().contains(player_entity[0]));
  EXPECT_FALSE(registry.storage<bim::game::dead>().contains(player_entity[1]));
  EXPECT_FALSE(player_animations.is_alive(player_state[0]->model));
  EXPECT_FALSE(player_animations.is_alive(player_state[1]->model));
}
