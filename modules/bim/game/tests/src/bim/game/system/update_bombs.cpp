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
#include <bim/game/system/update_bombs.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/bomb.hpp>
#include <bim/game/factory/brick_wall.hpp>

#include <entt/entity/registry.hpp>

#include <string>
#include <vector>

#include <gtest/gtest.h>

static std::vector<std::string> flames_map(const bim::game::arena& arena,
                                           const entt::registry& registry)
{
  std::vector<std::string> result(arena.height(),
                                  std::string(arena.width(), ' '));

  registry.view<bim::game::flame, bim::game::position_on_grid>().each(
      [&](entt::entity e, bim::game::flame f,
          bim::game::position_on_grid p) -> void
      {
        EXPECT_TRUE(arena.entity_at(p.x, p.y) == e);
        EXPECT_EQ(' ', result[p.y][p.x]);

        if (f.segment == bim::game::flame_segment::origin)
          result[p.y][p.x] = 'B';
        else if (bim::game::is_horizontal(f.direction))
          {
            if (f.segment == bim::game::flame_segment::tip)
              result[p.y][p.x] = 'h';
            else
              result[p.y][p.x] = 'H';
          }
        else if (f.segment == bim::game::flame_segment::tip)
          result[p.y][p.x] = 'v';
        else
          result[p.y][p.x] = 'V';
      });

  return result;
}

TEST(update_bombs, delay)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  constexpr std::uint8_t x = 0;
  constexpr std::uint8_t y = 0;
  constexpr std::uint8_t strength = 0;
  constexpr std::uint8_t player_index = 0;

  const entt::entity entity = bim::game::bomb_factory(
      registry, x, y, strength, player_index, std::chrono::milliseconds(24));
  bim::game::bomb& bomb = registry.get<bim::game::bomb>(entity);

  EXPECT_EQ(std::chrono::milliseconds(24), bomb.duration_until_explosion);

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(12), bomb.duration_until_explosion);
}

TEST(update_bombs, explode_strength_2)
{
  entt::registry registry;
  bim::game::arena arena(5, 3);
  const std::uint8_t bomb_x = arena.width() / 2;
  const std::uint8_t bomb_y = arena.height() / 2;
  const std::uint8_t strength = 2;
  constexpr std::uint8_t player_index = 0;

  const entt::entity entity =
      bim::game::bomb_factory(registry, bomb_x, bomb_y, strength, player_index,
                              std::chrono::milliseconds(24));

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_TRUE(registry.storage<bim::game::bomb>().contains(entity));

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_FALSE(registry.storage<bim::game::bomb>().contains(entity));

  const std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("  V  ", flames[0]);
  EXPECT_EQ("hHBHh", flames[1]);
  EXPECT_EQ("  V  ", flames[2]);
}

TEST(update_bombs, explode_strength_5)
{
  entt::registry registry;
  bim::game::arena arena(11, 11);
  const std::uint8_t bomb_x = arena.width() / 2;
  const std::uint8_t bomb_y = arena.height() / 2;
  const std::uint8_t strength = 5;
  constexpr std::uint8_t player_index = 0;

  bim::game::bomb_factory(registry, bomb_x, bomb_y, strength, player_index,
                          std::chrono::milliseconds(0));

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(24));

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
  bim::game::arena arena(8, 7);
  constexpr std::uint8_t strength = 2;
  constexpr std::uint8_t player_index = 0;

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
                   bim::game::bomb_factory(registry, 3, 3, strength,
                                           player_index,
                                           std::chrono::milliseconds(10)));
  arena.put_entity(4, 3,
                   bim::game::bomb_factory(registry, 4, 3, strength,
                                           player_index,
                                           std::chrono::milliseconds(200)));
  arena.put_entity(6, 3,
                   bim::game::bomb_factory(registry, 6, 3, strength,
                                           player_index,
                                           std::chrono::milliseconds(200)));
  arena.put_entity(4, 5,
                   bim::game::bomb_factory(registry, 4, 5, strength,
                                           player_index,
                                           std::chrono::milliseconds(200)));
  arena.put_entity(6, 5,
                   bim::game::bomb_factory(registry, 6, 5, strength,
                                           player_index,
                                           std::chrono::milliseconds(200)));

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   v    ", flames[1]);
  EXPECT_EQ("   V    ", flames[2]);
  EXPECT_EQ(" hHB    ", flames[3]);
  EXPECT_EQ("   V    ", flames[4]);
  EXPECT_EQ("   v    ", flames[5]);
  EXPECT_EQ("        ", flames[6]);

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   vv   ", flames[1]);
  EXPECT_EQ("   VV   ", flames[2]);
  EXPECT_EQ(" hHBBH  ", flames[3]);
  EXPECT_EQ("   VV   ", flames[4]);
  EXPECT_EQ("   v    ", flames[5]);
  EXPECT_EQ("        ", flames[6]);

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

  flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   vv v ", flames[1]);
  EXPECT_EQ("   VV V ", flames[2]);
  EXPECT_EQ(" hHBBHBH", flames[3]);
  EXPECT_EQ("   VV V ", flames[4]);
  EXPECT_EQ("   vBH  ", flames[5]);
  EXPECT_EQ("    V   ", flames[6]);

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(10));

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
  bim::game::arena arena(6, 6);
  constexpr std::uint8_t x = 2;
  constexpr std::uint8_t y = 2;
  constexpr std::uint8_t strength = 2;
  constexpr std::uint8_t player_index = 0;

  bim::game::bomb_factory(registry, x, y, strength, player_index,
                          std::chrono::milliseconds(0));

  const entt::entity walls[] = {
    bim::game::brick_wall_factory(registry, arena, 2, 1),
    bim::game::brick_wall_factory(registry, arena, 4, 2),
    bim::game::brick_wall_factory(registry, arena, 5, 2),
    bim::game::brick_wall_factory(registry, arena, 2, 5)
  };

  bim::game::update_bombs(registry, arena, std::chrono::milliseconds(12));

  EXPECT_TRUE(registry.storage<bim::game::burning>().contains(walls[0]));
  EXPECT_TRUE(registry.storage<bim::game::burning>().contains(walls[1]));
  EXPECT_FALSE(registry.storage<bim::game::burning>().contains(walls[2]));
  EXPECT_FALSE(registry.storage<bim::game::burning>().contains(walls[3]));
}
