/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bm/game/arena.hpp>
#include <bm/game/brick_wall.hpp>
#include <bm/game/level_generation.hpp>
#include <bm/game/player.hpp>
#include <bm/game/player_direction.hpp>
#include <bm/game/position_on_grid.hpp>
#include <bm/game/random_generator.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bm_game_level_generation_test
  : public ::testing::TestWithParam<std::tuple<int, int>>
{};

TEST_P(bm_game_level_generation_test, basic_solid_structure)
{
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bm::game::arena arena(width, height);

  bm::game::generate_basic_level_structure(arena);

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

TEST_P(bm_game_level_generation_test, random_brick_walls)
{
  const uint8_t width = std::get<0>(GetParam());
  const uint8_t height = std::get<1>(GetParam());
  bm::game::arena arena(width, height);

  bm::game::generate_basic_level_structure(arena);

  entt::registry registry;
  bm::game::random_generator random(1234);
  bm::game::insert_random_brick_walls(arena, registry, random, 50);

  int free_cell_count = 0;

  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      {
        const entt::entity entity = arena.entity_at(x, y);

        if(arena.is_static_wall(x, y))
          EXPECT_TRUE(entt::null == entity);
        else
          ++free_cell_count;
      }

  int brick_wall_count = 0;

  registry.view<bm::game::position_on_grid, bm::game::brick_wall>().each(
      [&brick_wall_count](const bm::game::position_on_grid&) -> void
      {
        ++brick_wall_count;
      });

  EXPECT_LT(0, brick_wall_count);
  EXPECT_LE(brick_wall_count, free_cell_count);
}

INSTANTIATE_TEST_CASE_P(bm_game_arena_suite, bm_game_level_generation_test,
                        ::testing::Combine(::testing::Range(3, 10),
                                           ::testing::Range(3, 10)));

TEST(bm_game_insert_random_brick_walls, no_walls_near_player)
{
  // insert_random_brick_walls should not create brick_walls around the
  // players.

  const uint8_t width = 10;
  const uint8_t height = 10;
  bm::game::arena arena(width, height);

  entt::registry registry;

  // Insert one player in each corner, and one in the center.
  const entt::entity player_entities[]
      = { registry.create(), registry.create(), registry.create(),
          registry.create(), registry.create() };
  const bm::game::position_on_grid player_positions[]
      = { { 1, 1 },
          { width - 2, 1 },
          { width / 2, height / 2 },
          { 1, height - 2 },
          { width - 2, height - 2 } };

  // Create the actual player entities that should be used by
  // insert_random_brick_walls to avoid creating walls.
  int i = 0;
  for(entt::entity e : player_entities)
    {
      registry.emplace<bm::game::player>(e, player_positions[i].x,
                                         player_positions[i].y,
                                         bm::game::player_direction::down);
      ++i;
    }

  bm::game::random_generator random(1234);

  // Insert brick walls with a 100% probability, i.e. create a wall every time.
  bm::game::insert_random_brick_walls(arena, registry, random, 100);

  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      {
        bool near_player_position = false;

        for(bm::game::position_on_grid p : player_positions)
          if((std::abs(p.x - x) <= 1) && (std::abs(p.y - y) <= 1))
            {
              near_player_position = true;
              break;
            }

        const entt::entity entity_in_arena = arena.entity_at(x, y);

        if(near_player_position)
          EXPECT_TRUE(entt::null == entity_in_arena);
        else
          EXPECT_TRUE(registry.storage<bm::game::brick_wall>().contains(
              entity_in_arena));
      }
}
