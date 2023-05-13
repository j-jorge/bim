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
#include <bm/game/system/update_bombs.hpp>

#include <bm/game/arena.hpp>

#include <bm/game/component/bomb.hpp>
#include <bm/game/component/burning.hpp>
#include <bm/game/component/flame.hpp>
#include <bm/game/component/flame_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>
#include <bm/game/factory/bomb.hpp>
#include <bm/game/factory/brick_wall.hpp>

#include <entt/entity/registry.hpp>

#include <string>
#include <vector>

#include <gtest/gtest.h>

static std::vector<std::string> flames_map(const bm::game::arena& arena,
                                           const entt::registry& registry)
{
  std::vector<std::string> result(arena.height(),
                                  std::string(arena.width(), ' '));

  registry.view<bm::game::flame, bm::game::position_on_grid>().each(
      [&](entt::entity e, bm::game::flame f,
          bm::game::position_on_grid p) -> void
      {
        EXPECT_TRUE(arena.entity_at(p.x, p.y) == e);
        EXPECT_EQ(' ', result[p.y][p.x]);

        EXPECT_TRUE((f.horizontal == bm::game::flame_horizontal::yes)
                    || (f.vertical == bm::game::flame_vertical::yes));

        if (f.horizontal == bm::game::flame_horizontal::yes)
          if (f.vertical == bm::game::flame_vertical::yes)
            result[p.y][p.x] = 'B';
          else if (f.end == bm::game::flame_end::yes)
            result[p.y][p.x] = 'h';
          else
            result[p.y][p.x] = 'H';
        else if (f.end == bm::game::flame_end::yes)
          result[p.y][p.x] = 'v';
        else
          result[p.y][p.x] = 'V';
      });

  return result;
}

TEST(update_bombs, delay)
{
  entt::registry registry;
  bm::game::arena arena(3, 3);

  const entt::entity entity = bm::game::bomb_factory(
      registry, 0, 0, 0, std::chrono::milliseconds(24));
  bm::game::bomb& bomb = registry.get<bm::game::bomb>(entity);

  EXPECT_EQ(std::chrono::milliseconds(24), bomb.duration_until_explosion);

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(12), bomb.duration_until_explosion);
}

TEST(update_bombs, explode_strength_2)
{
  entt::registry registry;
  bm::game::arena arena(5, 3);
  const std::uint8_t bomb_x = arena.width() / 2;
  const std::uint8_t bomb_y = arena.height() / 2;
  const std::uint8_t strength = 2;

  const entt::entity entity = bm::game::bomb_factory(
      registry, bomb_x, bomb_y, strength, std::chrono::milliseconds(24));

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_TRUE(registry.storage<bm::game::bomb>().contains(entity));

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_FALSE(registry.storage<bm::game::bomb>().contains(entity));

  const std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("  V  ", flames[0]);
  EXPECT_EQ("hHBHh", flames[1]);
  EXPECT_EQ("  V  ", flames[2]);
}

TEST(update_bombs, explode_strength_5)
{
  entt::registry registry;
  bm::game::arena arena(11, 11);
  const std::uint8_t bomb_x = arena.width() / 2;
  const std::uint8_t bomb_y = arena.height() / 2;
  const std::uint8_t strength = 5;

  bm::game::bomb_factory(registry, bomb_x, bomb_y, strength,
                         std::chrono::milliseconds(0));

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(24));

  const std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("     v     ", flames[0]);
  EXPECT_EQ("     V     ", flames[1]);
  EXPECT_EQ("     V     ", flames[2]);
  EXPECT_EQ("     V     ", flames[3]);
  EXPECT_EQ("     V     ", flames[4]);
  EXPECT_EQ("hHHHHBHHHHh", flames[5]);
  EXPECT_EQ("     V     ", flames[6]);
  EXPECT_EQ("     V     ", flames[7]);
  EXPECT_EQ("     V     ", flames[8]);
  EXPECT_EQ("     V     ", flames[9]);
  EXPECT_EQ("     v     ", flames[10]);
}

TEST(update_bombs, chain_reaction)
{
  entt::registry registry;
  bm::game::arena arena(8, 7);
  const std::uint8_t strength = 2;

  /*
    Bombs:
    .........
    .........
    .........
    ...xx.x..
    .........
    ....x.x..
    .........
  */
  arena.put_entity(3, 3,
                   bm::game::bomb_factory(registry, 3, 3, strength,
                                          std::chrono::milliseconds(10)));
  arena.put_entity(4, 3,
                   bm::game::bomb_factory(registry, 4, 3, strength,
                                          std::chrono::milliseconds(200)));
  arena.put_entity(6, 3,
                   bm::game::bomb_factory(registry, 6, 3, strength,
                                          std::chrono::milliseconds(200)));
  arena.put_entity(4, 5,
                   bm::game::bomb_factory(registry, 4, 5, strength,
                                          std::chrono::milliseconds(200)));
  arena.put_entity(6, 5,
                   bm::game::bomb_factory(registry, 6, 5, strength,
                                          std::chrono::milliseconds(200)));

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   v    ", flames[1]);
  EXPECT_EQ("   V    ", flames[2]);
  EXPECT_EQ(" hHB    ", flames[3]);
  EXPECT_EQ("   V    ", flames[4]);
  EXPECT_EQ("   v    ", flames[5]);
  EXPECT_EQ("        ", flames[6]);

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   vv   ", flames[1]);
  EXPECT_EQ("   VV   ", flames[2]);
  EXPECT_EQ(" hHBBH  ", flames[3]);
  EXPECT_EQ("   VV   ", flames[4]);
  EXPECT_EQ("   v    ", flames[5]);
  EXPECT_EQ("        ", flames[6]);

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   vv v ", flames[1]);
  EXPECT_EQ("   VV V ", flames[2]);
  EXPECT_EQ(" hHBBHBH", flames[3]);
  EXPECT_EQ("   VV V ", flames[4]);
  EXPECT_EQ("   vBH  ", flames[5]);
  EXPECT_EQ("    V   ", flames[6]);

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   vv v ", flames[1]);
  EXPECT_EQ("   VV V ", flames[2]);
  EXPECT_EQ(" hHBBHBH", flames[3]);
  EXPECT_EQ("   VV V ", flames[4]);
  EXPECT_EQ("   vBHBH", flames[5]);
  EXPECT_EQ("    V V ", flames[6]);
}

TEST(update_bombs, burning_walls)
{
  entt::registry registry;
  bm::game::arena arena(6, 6);

  bm::game::bomb_factory(registry, 2, 2, 2, std::chrono::milliseconds(0));

  const entt::entity walls[]
      = { bm::game::brick_wall_factory(registry, arena, 2, 1),
          bm::game::brick_wall_factory(registry, arena, 4, 2),
          bm::game::brick_wall_factory(registry, arena, 5, 2),
          bm::game::brick_wall_factory(registry, arena, 2, 5) };

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));

  EXPECT_TRUE(registry.storage<bm::game::burning>().contains(walls[0]));
  EXPECT_TRUE(registry.storage<bm::game::burning>().contains(walls[1]));
  EXPECT_FALSE(registry.storage<bm::game::burning>().contains(walls[2]));
  EXPECT_FALSE(registry.storage<bm::game::burning>().contains(walls[3]));
}
