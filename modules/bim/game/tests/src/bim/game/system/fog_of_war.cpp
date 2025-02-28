// SPDX-License-Identifier: AGPL-3.0-only
// #include <bim/game/system/update_fog_of_war.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/fog_of_war.hpp>
#include <bim/game/factory/player.hpp>

#include <bim/table_2d.impl.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

namespace bim
{
  namespace game
  {
    std::ostream& operator<<(std::ostream& os, bim::game::cell_neighborhood n)
    {
      const char* const names[] = { "left",      "right",     "up",
                                    "down",      "up_left",   "up_right",
                                    "down_left", "down_right" };

      const bim::game::cell_neighborhood values[] = {
        bim::game::cell_neighborhood::left,
        bim::game::cell_neighborhood::right,
        bim::game::cell_neighborhood::up,
        bim::game::cell_neighborhood::down,
        bim::game::cell_neighborhood::up_left,
        bim::game::cell_neighborhood::up_right,
        bim::game::cell_neighborhood::down_left,
        bim::game::cell_neighborhood::down_right
      };

      const char* separator = "[";

      for (std::size_t i = 0; i != std::size(names); ++i)
        if (!!(n & values[i]))
          {
            os << separator << ' ' << names[i] << ' ';
            separator = "|";
          }

      return os << ']';
    }
  }
}

static bim::table_2d<bim::game::cell_neighborhood>
fog_map(const bim::game::arena& arena, const entt::registry& registry)
{
  bim::table_2d<bim::game::cell_neighborhood> result(
      arena.width(), arena.height(), bim::game::cell_neighborhood::none);
  bim::table_2d<bool> seen(arena.width(), arena.height(), false);

  registry.view<bim::game::fog_of_war, bim::game::position_on_grid>().each(
      [&](const bim::game::fog_of_war& f,
          const bim::game::position_on_grid& p) -> void
      {
        EXPECT_FALSE(seen(p.x, p.y));
        seen(p.x, p.y) = true;

        result(p.x, p.y) = f.neighborhood;
      });

  return result;
}

TEST(fog_of_war, initial_fog)
{
  entt::registry registry;
  const bim::game::arena arena(5, 7);
  constexpr int player_0_x = 0;
  constexpr int player_0_y = 0;
  constexpr int player_1_x = 2;
  constexpr int player_1_y = 4;

  /*
      Players  Fog
    0 0....    fffff
    1 .....    fffff
    2 .....    fffff
    3 .....    f...f
    4 ..1..    f...f
    5 .....    f...f
    6 .....    fffff
  */
  bim::game::player_factory(registry, 0, player_0_x, player_0_y);
  bim::game::player_factory(registry, 1, player_1_x, player_1_y);

  bim::game::fog_of_war_factory(registry, 1, arena.width(), arena.height(),
                                {});

  const bim::table_2d<bim::game::cell_neighborhood> neighborhood =
      fog_map(arena, registry);

  // Row 0.
  EXPECT_EQ(bim::game::cell_neighborhood::right
                | bim::game::cell_neighborhood::down_right
                | bim::game::cell_neighborhood::down,
            neighborhood(0, 0));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::up
              | bim::game::cell_neighborhood::up_right),
            neighborhood(1, 0));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::up
              | bim::game::cell_neighborhood::up_right),
            neighborhood(2, 0));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::up
              | bim::game::cell_neighborhood::up_right),
            neighborhood(3, 0));
  EXPECT_EQ(bim::game::cell_neighborhood::left
                | bim::game::cell_neighborhood::down_left
                | bim::game::cell_neighborhood::down,
            neighborhood(4, 0));

  // Row 1.
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::left
              | bim::game::cell_neighborhood::down_left),
            neighborhood(0, 1));
  EXPECT_EQ(bim::game::cell_neighborhood::all, neighborhood(1, 1));
  EXPECT_EQ(bim::game::cell_neighborhood::all, neighborhood(2, 1));
  EXPECT_EQ(bim::game::cell_neighborhood::all, neighborhood(3, 1));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_right
              | bim::game::cell_neighborhood::right
              | bim::game::cell_neighborhood::down_right),
            neighborhood(4, 1));

  // Row 2.
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::up_right
                | bim::game::cell_neighborhood::right
                | bim::game::cell_neighborhood::down,
            neighborhood(0, 2));
  EXPECT_EQ(~(bim::game::cell_neighborhood::down
              | bim::game::cell_neighborhood::down_right),
            neighborhood(1, 2));
  EXPECT_EQ(~(bim::game::cell_neighborhood::down_left
              | bim::game::cell_neighborhood::down
              | bim::game::cell_neighborhood::down_right),
            neighborhood(2, 2));
  EXPECT_EQ(~(bim::game::cell_neighborhood::down_left
              | bim::game::cell_neighborhood::down),
            neighborhood(3, 2));
  EXPECT_EQ(bim::game::cell_neighborhood::up_left
                | bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::left
                | bim::game::cell_neighborhood::down,
            neighborhood(4, 2));

  // Row 3.
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::up_right
                | bim::game::cell_neighborhood::down,
            neighborhood(0, 3));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(1, 3));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(2, 3));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(3, 3));
  EXPECT_EQ(bim::game::cell_neighborhood::up_left
                | bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::down,
            neighborhood(4, 3));

  // Row 4.
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::down,
            neighborhood(0, 4));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(1, 4));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(2, 4));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(3, 4));
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::down,
            neighborhood(4, 4));

  // Row 5.
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::down
                | bim::game::cell_neighborhood::down_right,
            neighborhood(0, 5));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(1, 5));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(2, 5));
  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(3, 5));
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::down_left
                | bim::game::cell_neighborhood::down,
            neighborhood(4, 5));

  // Row 6.
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::right,
            neighborhood(0, 6));
  EXPECT_EQ(bim::game::cell_neighborhood::up_left
                | bim::game::cell_neighborhood::left
                | bim::game::cell_neighborhood::right,
            neighborhood(1, 6));
  EXPECT_EQ(bim::game::cell_neighborhood::left
                | bim::game::cell_neighborhood::right,
            neighborhood(2, 6));
  EXPECT_EQ(bim::game::cell_neighborhood::up_right
                | bim::game::cell_neighborhood::left
                | bim::game::cell_neighborhood::right,
            neighborhood(3, 6));
  EXPECT_EQ(bim::game::cell_neighborhood::up
                | bim::game::cell_neighborhood::left,
            neighborhood(4, 6));
}

TEST(fog_of_war, excluded)
{
  entt::registry registry;
  const bim::game::arena arena(5, 7);
  constexpr int player_0_x = 0;
  constexpr int player_0_y = 0;

  /*
      Players  Fog
    0 0....    ...ff
    1 .....    ...ff
    2 .....    ...ff
    3 .....    fffff
    4 .x...    f.fff
    5 ...x.    fff.f
    6 .....    fffff
  */
  bim::game::player_factory(registry, 0, player_0_x, player_0_y);

  const bim::game::position_on_grid excluded[] = { { 1, 4 }, { 3, 5 } };
  bim::game::fog_of_war_factory(registry, 0, arena.width(), arena.height(),
                                excluded);

  const bim::table_2d<bim::game::cell_neighborhood> neighborhood =
      fog_map(arena, registry);

  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(1, 4));
  EXPECT_EQ(~(bim::game::cell_neighborhood::down_right
              | bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::left
              | bim::game::cell_neighborhood::down_left),
            neighborhood(0, 3));
  EXPECT_EQ(~bim::game::cell_neighborhood::down, neighborhood(1, 3));
  EXPECT_EQ(~bim::game::cell_neighborhood::down_left, neighborhood(2, 3));
  EXPECT_EQ(~(bim::game::cell_neighborhood::right
              | bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::left
              | bim::game::cell_neighborhood::down_left),
            neighborhood(0, 4));
  EXPECT_EQ(~(bim::game::cell_neighborhood::left
              | bim::game::cell_neighborhood::down_right),
            neighborhood(2, 4));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_right
              | bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::left
              | bim::game::cell_neighborhood::down_left),
            neighborhood(0, 5));
  EXPECT_EQ(~bim::game::cell_neighborhood::up, neighborhood(1, 5));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::right),
            neighborhood(2, 5));

  EXPECT_EQ(bim::game::cell_neighborhood::none, neighborhood(3, 5));
  EXPECT_EQ(~(bim::game::cell_neighborhood::down_right
              | bim::game::cell_neighborhood::left),
            neighborhood(2, 4));
  EXPECT_EQ(~bim::game::cell_neighborhood::down, neighborhood(3, 4));
  EXPECT_EQ(~(bim::game::cell_neighborhood::down_left
              | bim::game::cell_neighborhood::up_right
              | bim::game::cell_neighborhood::right
              | bim::game::cell_neighborhood::down_right),
            neighborhood(4, 4));
  EXPECT_EQ(~(bim::game::cell_neighborhood::right
              | bim::game::cell_neighborhood::up_left),
            neighborhood(2, 5));
  EXPECT_EQ(~(bim::game::cell_neighborhood::left
              | bim::game::cell_neighborhood::up_right
              | bim::game::cell_neighborhood::right
              | bim::game::cell_neighborhood::down_right),
            neighborhood(4, 5));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_right
              | bim::game::cell_neighborhood::down_left
              | bim::game::cell_neighborhood::down
              | bim::game::cell_neighborhood::down_right),
            neighborhood(2, 6));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up
              | bim::game::cell_neighborhood::down_left
              | bim::game::cell_neighborhood::down
              | bim::game::cell_neighborhood::down_right),
            neighborhood(3, 6));
  EXPECT_EQ(~(bim::game::cell_neighborhood::up_left
              | bim::game::cell_neighborhood::down_left
              | bim::game::cell_neighborhood::down
              | bim::game::cell_neighborhood::down_right
              | bim::game::cell_neighborhood::right
              | bim::game::cell_neighborhood::up_right),
            neighborhood(4, 6));
}
