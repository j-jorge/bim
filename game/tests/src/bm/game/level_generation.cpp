#include <bm/game/arena.hpp>
#include <bm/game/level_generation.hpp>
#include <bm/game/position_on_grid.hpp>
#include <bm/game/wall.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bm_game_level_generation_test
  : public ::testing::TestWithParam<std::tuple<int, int>>
{};

TEST_P(bm_game_level_generation_test, solid_structure)
{
  entt::registry registry;
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bm::game::arena arena(width, height);

  bm::game::generate_basic_level(registry, arena);

  // Top border: solid walls everywhere.
  for(int x = 0; x != width; ++x)
    EXPECT_TRUE(arena.is_static_wall(x, 0)) << "x=" << x;

  for(int y = 1; y != height - 1; ++y)
    {
      // Left border: solid walls everywhere.
      EXPECT_TRUE(arena.is_static_wall(0, y)) << "y=" << y;

      // Odd line: no wall.
      if(y % 2 == 1)
        for(int x = 1; x != width - 1; ++x)
          EXPECT_FALSE(arena.is_static_wall(x, y)) << "y=" << y << ", x=" << x;
      else
        // Even line: a wall every two cells.
        for(int x = 1; x != width - 1; ++x)
          if(x % 2 == 0)
            EXPECT_TRUE(arena.is_static_wall(x, y))
                << "y=" << y << ", x=" << x;
          else
            EXPECT_FALSE(arena.is_static_wall(x, y))
                << "y=" << y << ", x=" << x;

      // Right border: solid walls everywhere.
      EXPECT_TRUE(arena.is_static_wall(width - 1, y))
          << "width - 1=" << (width - 1);
    }

  // Bottom border: solid walls everywhere.
  for(int x = 0; x != width; ++x)
    EXPECT_TRUE(arena.is_static_wall(x, height - 1))
        << "height - 1=" << (height - 1) << "x=" << x;
}

INSTANTIATE_TEST_CASE_P(bm_game_arena_suite, bm_game_level_generation_test,
                        ::testing::Combine(::testing::Range(3, 10),
                                           ::testing::Range(3, 10)));
