#include <bm/game/arena.hpp>
#include <bm/game/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bm_game_arena_test
  : public ::testing::TestWithParam<std::tuple<int, int>>
{};

TEST_P(bm_game_arena_test, dimensions)
{
  entt::registry registry;
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  const bm::game::arena arena(registry, width, height);

  EXPECT_EQ(width, arena.width());
  EXPECT_EQ(height, arena.height());

  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      {
        const bm::game::position_on_grid p
            = registry.get<bm::game::position_on_grid>(arena.at(x, y));
        EXPECT_EQ(x, p.x);
        EXPECT_EQ(y, p.y);
      }
}

INSTANTIATE_TEST_CASE_P(bm_game_arena_suite, bm_game_arena_test,
                        ::testing::Combine(::testing::Range(0, 10),
                                           ::testing::Range(0, 10)));
