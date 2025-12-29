// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/level_generation.hpp>

#include <bim/game/component/crate.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <bim/game/factory/player.hpp>

#include <bim/game/random_generator.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bim_game_level_generation_test
  : public ::testing::TestWithParam<std::tuple<int, int>>
{};

TEST_P(bim_game_level_generation_test, basic_solid_structure)
{
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bim::game::arena arena(width, height);

  bim::game::generate_basic_level_structure(arena);

  // Top border: solid walls everywhere.
  for (int x = 0; x != width; ++x)
    EXPECT_TRUE(arena.is_static_wall(x, 0)) << "x=" << x;

  for (int y = 1; y != height - 1; ++y)
    {
      // Left border: solid walls everywhere.
      EXPECT_TRUE(arena.is_static_wall(0, y)) << "y=" << y;

      // Odd line: no wall.
      if (y % 2 == 1)
        for (int x = 1; x != width - 1; ++x)
          EXPECT_FALSE(arena.is_static_wall(x, y)) << "y=" << y << ", x=" << x;
      else
        // Even line: a wall every two cells.
        for (int x = 1; x != width - 1; ++x)
          if (x % 2 == 0)
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
  for (int x = 0; x != width; ++x)
    EXPECT_TRUE(arena.is_static_wall(x, height - 1))
        << "height - 1=" << (height - 1) << "x=" << x;
}

TEST_P(bim_game_level_generation_test, random_crates)
{
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bim::game::arena arena(width, height);

  bim::game::generate_basic_level_structure(arena);

  entt::registry registry;
  bim::game::random_generator random(1234);
  bim::game::insert_random_crates(arena, registry, random, 50, {});

  int free_cell_count = 0;

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      {
        const entt::entity entity = arena.entity_at(x, y);

        if (arena.is_static_wall(x, y))
          EXPECT_TRUE(entt::null == entity);
        else
          ++free_cell_count;
      }

  int crate_count = 0;

  registry.view<bim::game::position_on_grid, bim::game::crate>().each(
      [&crate_count](const bim::game::position_on_grid&) -> void
      {
        ++crate_count;
      });

  EXPECT_LT(0, crate_count);
  EXPECT_LE(crate_count, free_cell_count);
}

// Levels with a size of 4 or less in one dimension barely have enough room for
// a player, so we test larger sizes.
INSTANTIATE_TEST_SUITE_P(bim_game_arena_suite, bim_game_level_generation_test,
                         ::testing::Combine(::testing::Range(5, 15),
                                            ::testing::Range(5, 15)));

TEST(bim_game_insert_random_crates, no_walls_near_player)
{
  // insert_random_crates should not create crates around the
  // players.

  const uint8_t width = 10;
  const uint8_t height = 10;
  bim::game::arena arena(width, height);

  entt::registry registry;

  // Insert one player in each corner, and one in the center.
  const bim::game::position_on_grid player_positions[] = {
    { 1, 1 },
    { width - 2, 1 },
    { width / 2, height / 2 },
    { 1, height - 2 },
    { width - 2, height - 2 }
  };

  // Create the actual player entities that should be used by
  // insert_random_crates to avoid creating walls.
  int i = 0;
  for (bim::game::position_on_grid p : player_positions)
    {
      bim::game::player_factory(registry, i, p.x, p.y,
                                bim::game::animation_id{});
      ++i;
    }

  bim::game::random_generator random(1234);

  // Insert crates with a 100% probability, i.e. create a crate every time.
  bim::game::insert_random_crates(arena, registry, random, 100, {});

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      {
        bool near_player_position = false;

        for (bim::game::position_on_grid p : player_positions)
          if ((std::abs(p.x - x) <= 1) && (std::abs(p.y - y) <= 1))
            {
              near_player_position = true;
              break;
            }

        const entt::entity entity_in_arena = arena.entity_at(x, y);

        if (near_player_position)
          EXPECT_TRUE(entt::null == entity_in_arena);
        else
          EXPECT_TRUE(
              registry.storage<bim::game::crate>().contains(entity_in_arena));
      }
}
