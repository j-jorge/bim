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

#include <bm/game/component/position_on_grid.hpp>

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
