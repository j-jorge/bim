// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/cell_neighborhood.hpp>

#include <gtest/gtest.h>

TEST(bim_game_navigation_check, static_walls)
{
  constexpr int arena_width = 6;
  constexpr int arena_height = 6;
  bim::game::arena arena(arena_width, arena_height);

  /*
    ......
    .xxxx.
    .x..x.
    .x..x.
    .xxxx.
    ......
  */
  arena.set_static_wall(1, 1, bim::game::cell_neighborhood::none);
  arena.set_static_wall(2, 1, bim::game::cell_neighborhood::none);
  arena.set_static_wall(3, 1, bim::game::cell_neighborhood::none);
  arena.set_static_wall(4, 1, bim::game::cell_neighborhood::none);

  arena.set_static_wall(1, 2, bim::game::cell_neighborhood::none);
  arena.set_static_wall(4, 2, bim::game::cell_neighborhood::none);

  arena.set_static_wall(1, 3, bim::game::cell_neighborhood::none);
  arena.set_static_wall(4, 3, bim::game::cell_neighborhood::none);

  arena.set_static_wall(1, 4, bim::game::cell_neighborhood::none);
  arena.set_static_wall(2, 4, bim::game::cell_neighborhood::none);
  arena.set_static_wall(3, 4, bim::game::cell_neighborhood::none);
  arena.set_static_wall(4, 4, bim::game::cell_neighborhood::none);

  {
    bim::game::navigation_check nav;

    for (int y = 2; y != 4; ++y)
      for (int x = 2; x != 4; ++x)
        for (int iy = 2; iy != 4; ++iy)
          for (int ix = 2; ix != 4; ++ix)
            ASSERT_TRUE(nav.reachable(arena, x, y, ix, iy))
                << "x=" << x << ", y=" << y << ", ix=" << ix << ", iy=" << iy;
  }

  {
    bim::game::navigation_check nav;

    for (int x = 0; x != arena_width; ++x)
      {
        for (int ix = 0; ix != arena_width; ++ix)
          {
            ASSERT_TRUE(nav.reachable(arena, x, 0, ix, 0))
                << "x=" << x << ", ix=" << ix;
            ASSERT_TRUE(nav.reachable(arena, ix, 0, x, 0))
                << "x=" << x << ", ix=" << ix;

            ASSERT_TRUE(nav.reachable(arena, x, arena_height - 1, ix,
                                      arena_height - 1))
                << "x=" << x << ", ix=" << ix;
            ASSERT_TRUE(nav.reachable(arena, ix, arena_height - 1, x,
                                      arena_height - 1))
                << "x=" << x << ", ix=" << ix;

            ASSERT_TRUE(nav.reachable(arena, x, 0, ix, arena_height - 1))
                << "x=" << x << ", ix=" << ix;
            ASSERT_TRUE(nav.reachable(arena, ix, arena_height - 1, x, 0))
                << "x=" << x << ", ix=" << ix;
          }

        for (int y = 1; y != arena_height - 1; ++y)
          {
            ASSERT_TRUE(nav.reachable(arena, x, 0, 0, y))
                << "x=" << x << ", y=" << y;
            ASSERT_TRUE(nav.reachable(arena, x, 0, arena_width - 1, y))
                << "x=" << x << ", y=" << y;

            ASSERT_TRUE(nav.reachable(arena, 0, y, x, 0))
                << "x=" << x << ", y=" << y;
            ASSERT_TRUE(nav.reachable(arena, arena_width - 1, y, x, 0))
                << "x=" << x << ", y=" << y;

            ASSERT_TRUE(nav.reachable(arena, x, arena_height - 1, 0, y))
                << "x=" << x << ", y=" << y;
            ASSERT_TRUE(
                nav.reachable(arena, x, arena_height - 1, arena_width - 1, y))
                << "x=" << x << ", y=" << y;

            ASSERT_TRUE(nav.reachable(arena, 0, y, x, arena_height - 1))
                << "x=" << x << ", y=" << y;
            ASSERT_TRUE(
                nav.reachable(arena, arena_width - 1, y, x, arena_height - 1))
                << "x=" << x << ", y=" << y;
          }
      }

    for (int y = 1; y != arena_height - 1; ++y)
      {
        ASSERT_TRUE(nav.reachable(arena, 0, y, arena_width - 1, y))
            << "y=" << y;
        ASSERT_TRUE(nav.reachable(arena, arena_width - 1, y, 0, y))
            << "y=" << y;
      }
  }

  {
    bim::game::navigation_check nav;

    for (int x = 0; x != arena_width; ++x)
      for (int iy = 2; iy != 4; ++iy)
        for (int ix = 2; ix != 4; ++ix)
          {
            ASSERT_FALSE(nav.reachable(arena, x, 0, ix, iy))
                << "x=" << x << ", ix=" << ix << ", iy=" << iy;
            ASSERT_FALSE(nav.reachable(arena, ix, iy, x, 0))
                << "x=" << x << ", ix=" << ix << ", iy=" << iy;

            ASSERT_FALSE(nav.reachable(arena, x, arena_height - 1, ix, iy))
                << "x=" << x << ", ix=" << ix << ", iy=" << iy;
            ASSERT_FALSE(nav.reachable(arena, ix, iy, x, arena_height - 1))
                << "x=" << x << ", ix=" << ix << ", iy=" << iy;
          }

    for (int y = 1; y != arena_height - 1; ++y)
      for (int iy = 2; iy != 4; ++iy)
        for (int ix = 2; ix != 4; ++ix)
          {
            ASSERT_FALSE(nav.reachable(arena, 0, y, ix, iy))
                << "y=" << y << ", ix=" << ix << ", iy=" << iy;
            ASSERT_FALSE(nav.reachable(arena, ix, iy, 0, y))
                << "y=" << y << ", ix=" << ix << ", iy=" << iy;

            ASSERT_FALSE(nav.reachable(arena, arena_width - 1, y, ix, iy))
                << "=" << y << ", ix=" << ix << ", iy=" << iy;
            ASSERT_FALSE(nav.reachable(arena, ix, iy, arena_width - 1, y))
                << "y=" << y << ", ix=" << ix << ", iy=" << iy;
          }
  }
}

TEST(bim_game_navigation_check, fences)
{
  constexpr int arena_width = 6;
  constexpr int arena_height = 6;
  bim::game::arena arena(arena_width, arena_height);

  /*
    ......
    ..xx..
    .x..x.
    .x..x.
    ..xx..
    ......
  */
  arena.add_fence(2, 1, bim::game::cell_edge::down);
  arena.add_fence(3, 1, bim::game::cell_edge::down);

  arena.add_fence(1, 2, bim::game::cell_edge::right);
  arena.add_fence(4, 2, bim::game::cell_edge::left);

  arena.add_fence(1, 3, bim::game::cell_edge::right);
  arena.add_fence(4, 3, bim::game::cell_edge::left);

  arena.add_fence(2, 4, bim::game::cell_edge::up);
  arena.add_fence(3, 4, bim::game::cell_edge::up);

  {
    bim::game::navigation_check nav;

    for (int y = 2; y != 4; ++y)
      for (int x = 2; x != 4; ++x)
        for (int iy = 2; iy != 4; ++iy)
          for (int ix = 2; ix != 4; ++ix)
            ASSERT_TRUE(nav.reachable(arena, x, y, ix, iy))
                << "x=" << x << ", y=" << y << ", ix=" << ix << ", iy=" << iy;
  }

  {
    bim::game::navigation_check nav;

    // top and bottom borders.
    for (int y = 0; y != 2; ++y)
      for (int x = 0; x != arena_width; ++x)
        for (int iy = 0; iy != 2; ++iy)
          for (int ix = 0; ix != arena_width; ++ix)
            {
              ASSERT_TRUE(nav.reachable(arena, x, y, ix, iy))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_TRUE(nav.reachable(arena, ix, iy, x, y))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;

              ASSERT_TRUE(nav.reachable(arena, x, arena_height - y - 1, ix,
                                        arena_height - iy - 1))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_TRUE(nav.reachable(arena, ix, arena_height - iy - 1, x,
                                        arena_height - y - 1))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;

              ASSERT_TRUE(
                  nav.reachable(arena, x, y, ix, arena_height - iy - 1))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_TRUE(
                  nav.reachable(arena, ix, arena_height - iy - 1, x, y))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
            }

    for (int y = 2; y != arena_height - 2; ++y)
      for (int x = 0; x != 2; ++x)
        {
          // left and right borders.
          for (int iy = 2; iy != arena_height - 2; ++iy)
            for (int ix = 0; ix != 2; ++ix)
              {
                ASSERT_TRUE(nav.reachable(arena, x, y, ix, iy))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(nav.reachable(arena, ix, iy, x, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(
                    nav.reachable(arena, x, y, arena_width - ix - 1, iy))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(
                    nav.reachable(arena, arena_width - ix - 1, iy, x, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(nav.reachable(arena, arena_width - x - 1, y,
                                          arena_width - ix - 1, iy))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(nav.reachable(arena, arena_width - ix - 1, iy,
                                          arena_width - x - 1, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
              }

          // vertical and horizontal borders
          for (int iy = 0; iy != 2; ++iy)
            for (int ix = 0; ix != arena_width; ++ix)
              {
                ASSERT_TRUE(nav.reachable(arena, x, y, ix, iy))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(nav.reachable(arena, ix, iy, x, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(
                    nav.reachable(arena, x, y, ix, arena_height - iy - 1))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(
                    nav.reachable(arena, ix, arena_height - iy - 1, x, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;

                ASSERT_TRUE(
                    nav.reachable(arena, arena_width - x - 1, y, ix, iy))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(
                    nav.reachable(arena, ix, iy, arena_width - x - 1, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(nav.reachable(arena, arena_width - x - 1, y, ix,
                                          arena_height - iy - 1))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
                ASSERT_TRUE(nav.reachable(arena, ix, arena_height - iy - 1,
                                          arena_width - x - 1, y))
                    << "x=" << x << ", y=" << y << ", ix=" << ix
                    << ", iy=" << iy;
              }
        }
  }

  {
    bim::game::navigation_check nav;

    // Inside and top/bottom.
    for (int y = 0; y != 2; ++y)
      for (int x = 0; x != arena_width; ++x)
        for (int iy = 2; iy != 4; ++iy)
          for (int ix = 2; ix != 4; ++ix)
            {
              ASSERT_FALSE(nav.reachable(arena, x, y, ix, iy))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_FALSE(nav.reachable(arena, ix, iy, x, y))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;

              ASSERT_FALSE(
                  nav.reachable(arena, x, arena_height - y - 1, ix, iy))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_FALSE(
                  nav.reachable(arena, ix, iy, x, arena_height - y - 1))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
            }

    // Inside and left/right.
    for (int y = 2; y != arena_height - 2; ++y)
      for (int x = 0; x != 2; ++x)
        for (int iy = 2; iy != 4; ++iy)
          for (int ix = 2; ix != 4; ++ix)
            {
              ASSERT_FALSE(nav.reachable(arena, x, y, ix, iy))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_FALSE(nav.reachable(arena, ix, iy, x, y))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;

              ASSERT_FALSE(
                  nav.reachable(arena, arena_width - x - 1, y, ix, iy))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
              ASSERT_FALSE(
                  nav.reachable(arena, ix, iy, arena_width - x - 1, y))
                  << "x=" << x << ", y=" << y << ", ix=" << ix
                  << ", iy=" << iy;
            }
  }
}
