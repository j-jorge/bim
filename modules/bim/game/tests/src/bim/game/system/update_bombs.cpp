// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_bombs.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/factory/bomb.hpp>
#include <bim/game/factory/brick_wall.hpp>
#include <bim/game/system/remove_dead_objects.hpp>
#include <bim/game/system/update_timers.hpp>

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
  bim::game::timer& timer = registry.get<bim::game::timer>(entity);

  EXPECT_EQ(std::chrono::milliseconds(24), timer.duration);
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

  bim::game::update_timers(registry, std::chrono::milliseconds(12));
  bim::game::update_bombs(registry, arena);
  EXPECT_TRUE(registry.storage<bim::game::bomb>().contains(entity));

  bim::game::update_timers(registry, std::chrono::milliseconds(12));
  bim::game::update_bombs(registry, arena);
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(entity));

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

  bim::game::update_timers(registry, std::chrono::milliseconds(24));
  bim::game::update_bombs(registry, arena);

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

TEST(update_bombs, stop_at_old_bomb)
{
  entt::registry registry;
  bim::game::arena arena(11, 11);
  constexpr std::uint8_t player_index = 0;

  bim::game::bomb_factory(registry, 2, 5, 1, player_index,
                          std::chrono::milliseconds(1));
  bim::game::bomb_factory(registry, 8, 5, 1, player_index,
                          std::chrono::milliseconds(1));
  bim::game::bomb_factory(registry, 5, 2, 1, player_index,
                          std::chrono::milliseconds(1));
  bim::game::bomb_factory(registry, 5, 8, 1, player_index,
                          std::chrono::milliseconds(1));
  bim::game::bomb_factory(registry, 5, 5, 11, player_index,
                          std::chrono::milliseconds(2));

  bim::game::update_timers(registry, std::chrono::milliseconds(1));
  bim::game::update_bombs(registry, arena);

  std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("           ", flames[0]);
  EXPECT_EQ("     v     ", flames[1]);
  EXPECT_EQ("    hBh    ", flames[2]);
  EXPECT_EQ("     v     ", flames[3]);
  EXPECT_EQ("  v     v  ", flames[4]);
  EXPECT_EQ(" hBh   hBh ", flames[5]);
  EXPECT_EQ("  v     v  ", flames[6]);
  EXPECT_EQ("     v     ", flames[7]);
  EXPECT_EQ("    hBh    ", flames[8]);
  EXPECT_EQ("     v     ", flames[9]);
  EXPECT_EQ("           ", flames[10]);

  bim::game::update_timers(registry, std::chrono::milliseconds(1));
  bim::game::update_bombs(registry, arena);
  flames = flames_map(arena, registry);
  EXPECT_EQ("           ", flames[0]);
  EXPECT_EQ("     v     ", flames[1]);
  EXPECT_EQ("    hBh    ", flames[2]);
  EXPECT_EQ("     v     ", flames[3]);
  EXPECT_EQ("  v  V  v  ", flames[4]);
  EXPECT_EQ(" hBhHBHhBh ", flames[5]);
  EXPECT_EQ("  v  V  v  ", flames[6]);
  EXPECT_EQ("     v     ", flames[7]);
  EXPECT_EQ("    hBh    ", flames[8]);
  EXPECT_EQ("     v     ", flames[9]);
  EXPECT_EQ("           ", flames[10]);
}

TEST(update_bombs, chain_reaction)
{
  entt::registry registry;
  bim::game::arena arena(8, 7);
  constexpr std::uint8_t strength = 2;
  constexpr std::uint8_t player_index = 0;

  /*
    Bombs:
    ........
    ........
    ........
    ...xx.x.
    ........
    ....x.x.
    ........
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

  bim::game::update_timers(registry, std::chrono::milliseconds(10));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   v    ", flames[1]);
  EXPECT_EQ("   V    ", flames[2]);
  EXPECT_EQ(" hHB    ", flames[3]);
  EXPECT_EQ("   V    ", flames[4]);
  EXPECT_EQ("   v    ", flames[5]);
  EXPECT_EQ("        ", flames[6]);

  bim::game::update_timers(registry, std::chrono::milliseconds(10));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  flames = flames_map(arena, registry);
  EXPECT_EQ("        ", flames[0]);
  EXPECT_EQ("   vv   ", flames[1]);
  EXPECT_EQ("   VV   ", flames[2]);
  EXPECT_EQ(" hHBBH  ", flames[3]);
  EXPECT_EQ("   VV   ", flames[4]);
  EXPECT_EQ("   v    ", flames[5]);
  EXPECT_EQ("        ", flames[6]);

  bim::game::update_timers(registry, std::chrono::milliseconds(10));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  {
    const char* const expected[] = { "        ", //
                                     "   vv v ", //
                                     "   VV V ", //
                                     " hHBBHBH", //
                                     "   VV V ", //
                                     "  hvBH  ", //
                                     "    V   " };
    const char* const variant_5 = "  hHBH  ";

    flames = flames_map(arena, registry);
    EXPECT_EQ(expected[0], flames[0]);
    EXPECT_EQ(expected[1], flames[1]);
    EXPECT_EQ(expected[2], flames[2]);
    EXPECT_EQ(expected[3], flames[3]);
    EXPECT_EQ(expected[4], flames[4]);
    EXPECT_TRUE((expected[5] == flames[5]) || (variant_5 == flames[5]))
        << "expected='" << expected[5] << "', variant='" << variant_5
        << "', flames='" << flames[5] << '\'';
    EXPECT_EQ(expected[6], flames[6]);
  }

  bim::game::update_timers(registry, std::chrono::milliseconds(10));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  {
    const char* const expected[] = { "        ", //
                                     "   vv v ", //
                                     "   VV V ", //
                                     " hHBBHBH", //
                                     "   VV V ", //
                                     "  hvBHBH", //
                                     "    V V " };
    const char* const variant_5 = "  hHBHBH";

    flames = flames_map(arena, registry);
    EXPECT_EQ(expected[0], flames[0]);
    EXPECT_EQ(expected[1], flames[1]);
    EXPECT_EQ(expected[2], flames[2]);
    EXPECT_EQ(expected[3], flames[3]);
    EXPECT_EQ(expected[4], flames[4]);
    EXPECT_TRUE((expected[5] == flames[5]) || (variant_5 == flames[5]))
        << "expected='" << expected[5] << "', variant='" << variant_5
        << "', flames='" << flames[5] << '\'';
    EXPECT_EQ(expected[6], flames[6]);
  }
}

TEST(update_bombs, flame_intersections_simultaneous)
{
  entt::registry registry;
  bim::game::arena arena(9, 7);
  constexpr std::uint8_t strength = 3;
  constexpr std::uint8_t player_index = 0;

  /*
    Bombs:
    .........
    .........
    .........
    ....x....
    .........
    ......x..
    .........
  */
  arena.put_entity(4, 3,
                   bim::game::bomb_factory(registry, 4, 3, strength,
                                           player_index,
                                           std::chrono::milliseconds(20)));
  arena.put_entity(6, 5,
                   bim::game::bomb_factory(registry, 6, 5, strength,
                                           player_index,
                                           std::chrono::milliseconds(20)));

  bim::game::update_timers(registry, std::chrono::milliseconds(20));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  const char* const expected[7] = { "    v    ", //
                                    "    V    ", //
                                    "    V v  ", //
                                    " hHHBHHh ", //
                                    "    V V  ", //
                                    "   hVHBHH", //
                                    "    v V  " };
  const char* const variant_3 = " hHHBHVh ";
  const char* const variant_5 = "   hHHBHH";

  const std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ(expected[0], flames[0]);
  EXPECT_EQ(expected[1], flames[1]);
  EXPECT_EQ(expected[2], flames[2]);
  EXPECT_TRUE((expected[3] == flames[3]) || (variant_3 == flames[3]))
      << "expected='" << expected[3] << "', variant=" << variant_3
      << "', flames='" << flames[3] << '\'';
  EXPECT_EQ(expected[4], flames[4]);
  EXPECT_TRUE((expected[5] == flames[5]) || (variant_5 == flames[5]))
      << "expected='" << expected[5] << "', variant='" << variant_5
      << "', flames='" << flames[5] << '\'';
  EXPECT_EQ(expected[6], flames[6]);
}

TEST(update_bombs, flame_intersections_sequential)
{
  entt::registry registry;
  bim::game::arena arena(9, 7);
  constexpr std::uint8_t strength = 3;
  constexpr std::uint8_t player_index = 0;

  /*
    Bombs:
    .........
    .........
    .........
    ....x....
    .........
    ......x..
    .........
  */
  arena.put_entity(4, 3,
                   bim::game::bomb_factory(registry, 4, 3, strength,
                                           player_index,
                                           std::chrono::milliseconds(20)));
  arena.put_entity(6, 5,
                   bim::game::bomb_factory(registry, 6, 5, strength,
                                           player_index,
                                           std::chrono::milliseconds(21)));

  bim::game::update_timers(registry, std::chrono::milliseconds(20));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  {
    const char* const expected[7] = { "    v    ", //
                                      "    V    ", //
                                      "    V    ", //
                                      " hHHBHHh ", //
                                      "    V    ", //
                                      "    V    ", //
                                      "    v    " };

    const std::vector<std::string> flames = flames_map(arena, registry);
    EXPECT_EQ(expected[0], flames[0]);
    EXPECT_EQ(expected[1], flames[1]);
    EXPECT_EQ(expected[2], flames[2]);
    EXPECT_EQ(expected[3], flames[3]);
    EXPECT_EQ(expected[4], flames[4]);
    EXPECT_EQ(expected[5], flames[5]);
    EXPECT_EQ(expected[6], flames[6]);
  }

  bim::game::update_timers(registry, std::chrono::milliseconds(1));
  bim::game::update_bombs(registry, arena);
  bim::game::remove_dead_objects(registry);

  {
    const char* const expected[7] = { "    v    ", //
                                      "    V    ", //
                                      "    V v  ", //
                                      " hHHBHHh ", //
                                      "    V V  ", //
                                      "   hVHBHH", //
                                      "    v V  " };
    const char* const variant_3 = " hHHBHVh";
    const char* const variant_5 = "   hHHBHH";

    const std::vector<std::string> flames = flames_map(arena, registry);
    EXPECT_EQ(expected[0], flames[0]);
    EXPECT_EQ(expected[1], flames[1]);
    EXPECT_EQ(expected[2], flames[2]);
    EXPECT_TRUE((expected[3] == flames[3]) || (variant_3 == flames[3]))
        << "expected='" << expected[3] << "', variant=" << variant_3
        << "', flames='" << flames[3] << '\'';
    EXPECT_EQ(expected[4], flames[4]);
    EXPECT_TRUE((expected[5] == flames[5]) || (variant_5 == flames[5]))
        << "expected='" << expected[5] << "', variant='" << variant_5
        << "', flames='" << flames[5] << '\'';
    EXPECT_EQ(expected[6], flames[6]);
  }
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

  bim::game::update_timers(registry, std::chrono::milliseconds(12));
  bim::game::update_bombs(registry, arena);

  EXPECT_TRUE(registry.storage<bim::game::burning>().contains(walls[0]));
  EXPECT_TRUE(registry.storage<bim::game::burning>().contains(walls[1]));
  EXPECT_FALSE(registry.storage<bim::game::burning>().contains(walls[2]));
  EXPECT_FALSE(registry.storage<bim::game::burning>().contains(walls[3]));
}
