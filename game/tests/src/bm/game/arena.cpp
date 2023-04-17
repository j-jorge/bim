#include <bm/game/arena.hpp>

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
}

INSTANTIATE_TEST_CASE_P(bm_game_arena_suite, bm_game_arena_test,
                        ::testing::Combine(::testing::Range(3, 20),
                                           ::testing::Range(3, 20)));
