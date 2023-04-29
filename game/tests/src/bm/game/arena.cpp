#include <bm/game/arena.hpp>
#include <bm/game/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bm_game_arena_test
  : public ::testing::TestWithParam<std::tuple<int, int>>
{};

TEST_P(bm_game_arena_test, defaults)
{
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bm::game::arena arena(width, height);

  EXPECT_EQ(width, arena.width());
  EXPECT_EQ(height, arena.height());

  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      EXPECT_FALSE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;

  arena.set_static_wall(0, 0);
  EXPECT_TRUE(arena.is_static_wall(0, 0));

  arena.set_static_wall(width / 2, 0);
  EXPECT_TRUE(arena.is_static_wall(width / 2, 0));

  arena.set_static_wall(0, height / 2);
  EXPECT_TRUE(arena.is_static_wall(0, height / 2));

  arena.set_static_wall(width / 2, height / 2);
  EXPECT_TRUE(arena.is_static_wall(width / 2, height / 2));

  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      if(((y == 0) && (x == 0)) || ((y == 0) && (x == width / 2))
         || ((y == height / 2) && (x == 0))
         || ((y == height / 2) && (x == width / 2)))
        EXPECT_TRUE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;
      else
        EXPECT_FALSE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;
}

INSTANTIATE_TEST_CASE_P(bm_game_arena_suite, bm_game_arena_test,
                        ::testing::Combine(::testing::Range(1, 10),
                                           ::testing::Range(1, 10)));

TEST(bm_game_arena, put_entity)
{
  bm::game::arena arena(2, 2);

  entt::registry registry;
  const entt::entity entity = registry.create();

  arena.put_entity(0, 1, entity);

  EXPECT_NE(entity, arena.entity_at(0, 0));
  EXPECT_EQ(entity, arena.entity_at(0, 1));
  EXPECT_NE(entity, arena.entity_at(1, 0));
  EXPECT_NE(entity, arena.entity_at(1, 1));
}
