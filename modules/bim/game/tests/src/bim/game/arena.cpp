// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/cell_edge.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/static_wall.hpp>

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

  arena.set_static_wall(0, 0, bim::game::cell_neighborhood::none);
  EXPECT_TRUE(arena.is_static_wall(0, 0));

  arena.set_static_wall(width / 2, 0, bim::game::cell_neighborhood::none);
  EXPECT_TRUE(arena.is_static_wall(width / 2, 0));

  arena.set_static_wall(0, height / 2, bim::game::cell_neighborhood::none);
  EXPECT_TRUE(arena.is_static_wall(0, height / 2));

  arena.set_static_wall(width / 2, height / 2,
                        bim::game::cell_neighborhood::none);
  EXPECT_TRUE(arena.is_static_wall(width / 2, height / 2));

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      {
        if (((y == 0) && (x == 0)) || ((y == 0) && (x == width / 2))
            || ((y == height / 2) && (x == 0))
            || ((y == height / 2) && (x == width / 2)))
          EXPECT_TRUE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;
        else
          EXPECT_FALSE(arena.is_static_wall(x, y)) << "x=" << x << ", y=" << y;

        EXPECT_EQ(bim::game::cell_edge::none, arena.fences(x, y))
            << "x=" << x << ", y=" << y;
      }
}

INSTANTIATE_TEST_SUITE_P(bim_game_arena_suite, bim_game_arena_test,
                         ::testing::Combine(::testing::Range(3, 10),
                                            ::testing::Range(3, 10)));

TEST(bim_game_arena, static_wall_view)
{
  bim::game::arena arena(2, 2);

  arena.set_static_wall(0, 1, bim::game::cell_neighborhood::left);
  arena.set_static_wall(1, 0, bim::game::cell_neighborhood::right);

  const std::span<const bim::game::static_wall> walls = arena.static_walls();

  EXPECT_EQ(0, walls[0].x);
  EXPECT_EQ(1, walls[0].y);
  EXPECT_EQ(bim::game::cell_neighborhood::left, walls[0].neighbors);

  EXPECT_EQ(1, walls[1].x);
  EXPECT_EQ(0, walls[1].y);
  EXPECT_EQ(bim::game::cell_neighborhood::right, walls[1].neighbors);
}

TEST(bim_game_arena, fences)
{
  bim::game::arena arena(2, 2);

  arena.add_fence(0, 1, bim::game::cell_edge::left);
  arena.add_fence(1, 0, bim::game::cell_edge::right);

  EXPECT_EQ(bim::game::cell_edge::left, arena.fences(0, 1));
  EXPECT_EQ(bim::game::cell_edge::right, arena.fences(1, 0));
}
