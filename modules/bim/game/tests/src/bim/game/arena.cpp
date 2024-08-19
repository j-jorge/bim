// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bim_game_arena_test
  : public ::testing::TestWithParam<std::tuple<int, int>>
{};

TEST_P(bim_game_arena_test, defaults)
{
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bim::game::arena arena(width, height);

  EXPECT_EQ(width, arena.width());
  EXPECT_EQ(height, arena.height());

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      EXPECT_FALSE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;

  arena.set_static_wall(0, 0);
  EXPECT_TRUE(arena.is_static_wall(0, 0));

  arena.set_static_wall(width / 2, 0);
  EXPECT_TRUE(arena.is_static_wall(width / 2, 0));

  arena.set_static_wall(0, height / 2);
  EXPECT_TRUE(arena.is_static_wall(0, height / 2));

  arena.set_static_wall(width / 2, height / 2);
  EXPECT_TRUE(arena.is_static_wall(width / 2, height / 2));

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      if (((y == 0) && (x == 0)) || ((y == 0) && (x == width / 2))
          || ((y == height / 2) && (x == 0))
          || ((y == height / 2) && (x == width / 2)))
        EXPECT_TRUE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;
      else
        EXPECT_FALSE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;
}

INSTANTIATE_TEST_CASE_P(bim_game_arena_suite, bim_game_arena_test,
                        ::testing::Combine(::testing::Range(1, 10),
                                           ::testing::Range(1, 10)));

TEST(bim_game_arena, static_wall_is_solid)
{
  bim::game::arena arena(2, 2);

  arena.set_static_wall(0, 1);

  EXPECT_FALSE(arena.is_solid(0, 0));
  EXPECT_TRUE(arena.is_solid(0, 1));
  EXPECT_FALSE(arena.is_solid(1, 0));
  EXPECT_FALSE(arena.is_solid(1, 1));
}

TEST(bim_game_arena, solid_not_static_wall)
{
  bim::game::arena arena(2, 2);

  arena.set_solid(0, 1);

  EXPECT_FALSE(arena.is_solid(0, 0));
  EXPECT_TRUE(arena.is_solid(0, 1));
  EXPECT_FALSE(arena.is_solid(1, 0));
  EXPECT_FALSE(arena.is_solid(1, 1));
}

TEST(bim_game_arena, put_entity)
{
  bim::game::arena arena(2, 2);

  entt::registry registry;
  const entt::entity entity = registry.create();

  arena.put_entity(0, 1, entity);

  EXPECT_NE(entity, arena.entity_at(0, 0));
  EXPECT_EQ(entity, arena.entity_at(0, 1));
  EXPECT_NE(entity, arena.entity_at(1, 0));
  EXPECT_NE(entity, arena.entity_at(1, 1));
}

TEST(bim_game_arena, erase_entity)
{
  bim::game::arena arena(2, 2);

  entt::registry registry;
  const entt::entity entities[] = { registry.create(), registry.create() };

  arena.put_entity(0, 1, entities[0]);
  arena.put_entity(1, 0, entities[1]);
  arena.set_solid(1, 0);

  EXPECT_TRUE(entt::null == arena.entity_at(0, 0));
  EXPECT_EQ(entities[0], arena.entity_at(0, 1));
  EXPECT_EQ(entities[1], arena.entity_at(1, 0));
  EXPECT_TRUE(entt::null == arena.entity_at(1, 1));
  EXPECT_TRUE(arena.is_solid(1, 0));

  arena.erase_entity(0, 1);

  EXPECT_TRUE(entt::null == arena.entity_at(0, 0));
  EXPECT_TRUE(entt::null == arena.entity_at(0, 1));
  EXPECT_EQ(entities[1], arena.entity_at(1, 0));
  EXPECT_TRUE(entt::null == arena.entity_at(1, 1));
  EXPECT_TRUE(arena.is_solid(1, 0));

  arena.erase_entity(1, 0);

  EXPECT_TRUE(entt::null == arena.entity_at(0, 0));
  EXPECT_TRUE(entt::null == arena.entity_at(0, 1));
  EXPECT_TRUE(entt::null == arena.entity_at(1, 0));
  EXPECT_TRUE(entt::null == arena.entity_at(1, 1));
  EXPECT_FALSE(arena.is_solid(1, 0));
}
