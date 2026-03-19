// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/wall.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_navigation_check, reachable_static_walls)
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

TEST(bim_game_navigation_check, reachable_fences)
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

TEST(bim_game_navigation_check, paths_in)
{
  constexpr int arena_width = 7;
  constexpr int arena_height = 7;
  bim::game::arena arena(arena_width, arena_height);

  entt::registry registry;
  bim::game::entity_world_map entity_map(arena_width, arena_height);

  /*
    x: static wall
    s: solid
    e: edge (on the inside of the inner area).
    f: not allowed

    .......
    .xexsx.
    .e...e.
    .x.f.x.
    .s...s.
    .xexsx.
    .......
  */
  arena.set_static_wall(1, 1, bim::game::cell_neighborhood::none);
  arena.add_fence(2, 1, bim::game::cell_edge::down);
  arena.set_static_wall(3, 1, bim::game::cell_neighborhood::none);
  bim::game::wall_factory(registry, entity_map, 4, 1);
  arena.set_static_wall(5, 1, bim::game::cell_neighborhood::none);

  arena.add_fence(1, 2, bim::game::cell_edge::right);
  arena.add_fence(5, 2, bim::game::cell_edge::left);

  arena.set_static_wall(1, 3, bim::game::cell_neighborhood::none);
  arena.set_static_wall(5, 3, bim::game::cell_neighborhood::none);

  bim::game::wall_factory(registry, entity_map, 1, 4);
  bim::game::wall_factory(registry, entity_map, 5, 4);

  arena.set_static_wall(1, 5, bim::game::cell_neighborhood::none);
  arena.add_fence(2, 5, bim::game::cell_edge::up);
  arena.set_static_wall(3, 5, bim::game::cell_neighborhood::none);
  bim::game::wall_factory(registry, entity_map, 4, 5);
  arena.set_static_wall(5, 5, bim::game::cell_neighborhood::none);

  bim::table_2d<bool> allowed(arena_width, arena_height, true);
  allowed(3, 3) = false;

  // Inside.
  bim::game::navigation_check nav;

  bim::table_2d<std::uint8_t> distance(arena_width, arena_height);
  bim::table_2d<bim::game::position_on_grid> previous(arena_width,
                                                      arena_height);

  nav.paths(distance, previous, registry, arena, entity_map, 2, 2, allowed);

  for (int x = 0; x != arena_width; ++x)
    {
      ASSERT_EQ(bim::game::navigation_check::unreachable, distance(x, 0))
          << "x=" << x;
      ASSERT_EQ(bim::game::navigation_check::unreachable,
                distance(x, arena_height - 1))
          << "x=" << x;
    }

  for (int y = 1; y != arena_height - 1; ++y)
    {
      ASSERT_EQ(bim::game::navigation_check::unreachable, distance(0, y))
          << "y=" << y;
      ASSERT_EQ(bim::game::navigation_check::unreachable,
                distance(arena_width - 1, y))
          << "y=" << y;
    }

  ASSERT_EQ(0, distance(2, 2));
  ASSERT_EQ(2, previous(2, 2).x);
  ASSERT_EQ(2, previous(2, 2).y);

  // Path to the right.
  ASSERT_EQ(1, distance(3, 2));
  ASSERT_EQ(2, previous(3, 2).x);
  ASSERT_EQ(2, previous(3, 2).y);

  ASSERT_EQ(2, distance(4, 2));
  ASSERT_EQ(3, previous(4, 2).x);
  ASSERT_EQ(2, previous(4, 2).y);

  ASSERT_EQ(3, distance(4, 3));
  ASSERT_EQ(4, previous(4, 3).x);
  ASSERT_EQ(2, previous(4, 3).y);

  ASSERT_EQ(4, distance(4, 4));
  ASSERT_EQ(4, previous(4, 4).x);
  ASSERT_EQ(3, previous(4, 4).y);

  // Path to the left.
  ASSERT_EQ(1, distance(2, 3));
  ASSERT_EQ(2, previous(2, 3).x);
  ASSERT_EQ(2, previous(2, 3).y);

  ASSERT_EQ(2, distance(2, 4));
  ASSERT_EQ(2, previous(2, 4).x);
  ASSERT_EQ(3, previous(2, 4).y);

  ASSERT_EQ(3, distance(3, 4));
  ASSERT_EQ(2, previous(3, 4).x);
  ASSERT_EQ(4, previous(3, 4).y);

  // Cells on the border of the reachable area.
  ASSERT_EQ(bim::game::navigation_check::border, distance(4, 1));
  ASSERT_EQ(bim::game::navigation_check::border, distance(3, 3));
  ASSERT_EQ(bim::game::navigation_check::border, distance(1, 4));
  ASSERT_EQ(bim::game::navigation_check::border, distance(5, 4));
  ASSERT_EQ(bim::game::navigation_check::border, distance(4, 5));

  // Free cells that cannot be reached due to a fence.
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(2, 1));
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(1, 2));
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(5, 2));
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(2, 5));
}

TEST(bim_game_navigation_check, paths_out)
{
  constexpr int arena_width = 7;
  constexpr int arena_height = 7;
  bim::game::arena arena(arena_width, arena_height);

  entt::registry registry;
  bim::game::entity_world_map entity_map(arena_width, arena_height);

  /*
    x: static wall
    s: solid
    e: edge (on the outside of the inner area).
    f: not allowed

    .......
    .xexsx.
    .e...e.
    .x.f.x.
    .s...s.
    .xexsx.
    .......
  */
  arena.set_static_wall(1, 1, bim::game::cell_neighborhood::none);
  arena.add_fence(2, 1, bim::game::cell_edge::up);
  arena.set_static_wall(3, 1, bim::game::cell_neighborhood::none);
  bim::game::wall_factory(registry, entity_map, 4, 1);
  arena.set_static_wall(5, 1, bim::game::cell_neighborhood::none);

  arena.add_fence(1, 2, bim::game::cell_edge::left);
  arena.add_fence(5, 2, bim::game::cell_edge::right);

  arena.set_static_wall(1, 3, bim::game::cell_neighborhood::none);
  arena.set_static_wall(5, 3, bim::game::cell_neighborhood::none);

  bim::game::wall_factory(registry, entity_map, 1, 4);
  bim::game::wall_factory(registry, entity_map, 5, 4);

  arena.set_static_wall(1, 5, bim::game::cell_neighborhood::none);
  arena.add_fence(2, 5, bim::game::cell_edge::down);
  arena.set_static_wall(3, 5, bim::game::cell_neighborhood::none);
  bim::game::wall_factory(registry, entity_map, 4, 5);
  arena.set_static_wall(5, 5, bim::game::cell_neighborhood::none);

  bim::table_2d<bool> allowed(arena_width, arena_height, true);

  // Inside.
  bim::game::navigation_check nav;

  bim::table_2d<std::uint8_t> distance(arena_width, arena_height);
  bim::table_2d<bim::game::position_on_grid> previous(arena_width,
                                                      arena_height);

  nav.paths(distance, previous, registry, arena, entity_map, 3, 0, allowed);

  for (int y = 2; y != arena_height - 2; ++y)
    for (int x = 2; x != arena_width - 2; ++x)
      ASSERT_EQ(bim::game::navigation_check::unreachable, distance(x, y))
          << "x=" << x << ", y=" << y;

  ASSERT_EQ(0, distance(3, 0));
  ASSERT_EQ(3, previous(3, 0).x);
  ASSERT_EQ(0, previous(3, 0).y);

  // Path to the right.
  ASSERT_EQ(1, distance(4, 0));
  ASSERT_EQ(3, previous(4, 0).x);
  ASSERT_EQ(0, previous(4, 0).y);

  ASSERT_EQ(2, distance(5, 0));
  ASSERT_EQ(4, previous(5, 0).x);
  ASSERT_EQ(0, previous(5, 0).y);

  ASSERT_EQ(3, distance(6, 0));
  ASSERT_EQ(5, previous(6, 0).x);
  ASSERT_EQ(0, previous(6, 0).y);

  ASSERT_EQ(4, distance(6, 1));
  ASSERT_EQ(6, previous(6, 1).x);
  ASSERT_EQ(0, previous(6, 1).y);

  ASSERT_EQ(5, distance(6, 2));
  ASSERT_EQ(6, previous(6, 2).x);
  ASSERT_EQ(1, previous(6, 2).y);

  ASSERT_EQ(6, distance(6, 3));
  ASSERT_EQ(6, previous(6, 3).x);
  ASSERT_EQ(2, previous(6, 3).y);

  ASSERT_EQ(7, distance(6, 4));
  ASSERT_EQ(6, previous(6, 4).x);
  ASSERT_EQ(3, previous(6, 4).y);

  ASSERT_EQ(8, distance(6, 5));
  ASSERT_EQ(6, previous(6, 5).x);
  ASSERT_EQ(4, previous(6, 5).y);

  ASSERT_EQ(9, distance(6, 6));
  ASSERT_EQ(6, previous(6, 6).x);
  ASSERT_EQ(5, previous(6, 6).y);

  ASSERT_EQ(10, distance(5, 6));
  ASSERT_EQ(6, previous(5, 6).x);
  ASSERT_EQ(6, previous(5, 6).y);

  ASSERT_EQ(11, distance(4, 6));
  ASSERT_EQ(5, previous(4, 6).x);
  ASSERT_EQ(6, previous(4, 6).y);

  ASSERT_EQ(12, distance(3, 6));
  // It may be (2, 6) or (4, 6).
  ASSERT_EQ(1, std::abs(3 - previous(3, 6).x));
  ASSERT_EQ(6, previous(3, 6).y);

  // Path to the left.
  ASSERT_EQ(1, distance(2, 0));
  ASSERT_EQ(3, previous(2, 0).x);
  ASSERT_EQ(0, previous(2, 0).y);

  ASSERT_EQ(2, distance(1, 0));
  ASSERT_EQ(2, previous(1, 0).x);
  ASSERT_EQ(0, previous(1, 0).y);

  ASSERT_EQ(3, distance(0, 0));
  ASSERT_EQ(1, previous(0, 0).x);
  ASSERT_EQ(0, previous(0, 0).y);

  ASSERT_EQ(4, distance(0, 1));
  ASSERT_EQ(0, previous(0, 1).x);
  ASSERT_EQ(0, previous(0, 1).y);

  ASSERT_EQ(5, distance(0, 2));
  ASSERT_EQ(0, previous(0, 2).x);
  ASSERT_EQ(1, previous(0, 2).y);

  ASSERT_EQ(6, distance(0, 3));
  ASSERT_EQ(0, previous(0, 3).x);
  ASSERT_EQ(2, previous(0, 3).y);

  ASSERT_EQ(7, distance(0, 4));
  ASSERT_EQ(0, previous(0, 4).x);
  ASSERT_EQ(3, previous(0, 4).y);

  ASSERT_EQ(8, distance(0, 5));
  ASSERT_EQ(0, previous(0, 5).x);
  ASSERT_EQ(4, previous(0, 5).y);

  ASSERT_EQ(9, distance(0, 6));
  ASSERT_EQ(0, previous(0, 6).x);
  ASSERT_EQ(5, previous(0, 6).y);

  ASSERT_EQ(10, distance(1, 6));
  ASSERT_EQ(0, previous(1, 6).x);
  ASSERT_EQ(6, previous(1, 6).y);

  ASSERT_EQ(11, distance(2, 6));
  ASSERT_EQ(1, previous(2, 6).x);
  ASSERT_EQ(6, previous(2, 6).y);

  // Cells on the border of the reachable area.
  ASSERT_EQ(bim::game::navigation_check::border, distance(4, 1));
  ASSERT_EQ(bim::game::navigation_check::border, distance(1, 4));
  ASSERT_EQ(bim::game::navigation_check::border, distance(5, 4));
  ASSERT_EQ(bim::game::navigation_check::border, distance(4, 5));

  // Free cells that cannot be reached due to a fence.
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(2, 1));
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(1, 2));
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(5, 2));
  ASSERT_EQ(bim::game::navigation_check::unreachable, distance(2, 5));
}

TEST(bim_game_navigation_check, exists)
{
  constexpr int arena_width = 7;
  constexpr int arena_height = 7;
  bim::game::arena arena(arena_width, arena_height);

  entt::registry registry;
  bim::game::entity_world_map entity_map(arena_width, arena_height);

  /*
    x: static wall
    s: solid
    e: edge (on the inside of the inner area).
    f: forbidden

    fffffff
    .xexsx.
    .efffe.
    .xfffx.
    .s...s.
    .xexsx.
    .......
  */
  arena.set_static_wall(1, 1, bim::game::cell_neighborhood::none);
  arena.add_fence(2, 1, bim::game::cell_edge::down);
  arena.set_static_wall(3, 1, bim::game::cell_neighborhood::none);
  bim::game::wall_factory(registry, entity_map, 4, 1);
  arena.set_static_wall(5, 1, bim::game::cell_neighborhood::none);

  arena.add_fence(1, 2, bim::game::cell_edge::right);
  arena.add_fence(5, 2, bim::game::cell_edge::left);

  arena.set_static_wall(1, 3, bim::game::cell_neighborhood::none);
  arena.set_static_wall(5, 3, bim::game::cell_neighborhood::none);

  bim::game::wall_factory(registry, entity_map, 1, 4);
  bim::game::wall_factory(registry, entity_map, 5, 4);

  arena.set_static_wall(1, 5, bim::game::cell_neighborhood::none);
  arena.add_fence(2, 5, bim::game::cell_edge::up);
  arena.set_static_wall(3, 5, bim::game::cell_neighborhood::none);
  bim::game::wall_factory(registry, entity_map, 4, 5);
  arena.set_static_wall(5, 5, bim::game::cell_neighborhood::none);

  bim::table_2d<bool> forbidden(arena_width, arena_height, false);
  for (int x = 0; x != arena_width; ++x)
    forbidden(x, 0) = true;

  for (int y = 2; y != 4; ++y)
    for (int x = 2; x != 5; ++x)
      forbidden(x, y) = true;

  bim::game::navigation_check nav;

  // Inside.
  ASSERT_FALSE(nav.exists(registry, arena, entity_map, 3, 2, 0, forbidden));
  ASSERT_FALSE(nav.exists(registry, arena, entity_map, 3, 2, 1, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 2, 2, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 2, 3, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 2, 50, forbidden));

  // Outside
  ASSERT_FALSE(nav.exists(registry, arena, entity_map, 3, 0, 0, forbidden));
  ASSERT_FALSE(nav.exists(registry, arena, entity_map, 3, 0, 1, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 0, 2, forbidden));

  // Forbid the cell with the edge on the opposite side.
  forbidden(2, 1) = true;

  ASSERT_FALSE(nav.exists(registry, arena, entity_map, 3, 0, 2, forbidden));
  ASSERT_FALSE(nav.exists(registry, arena, entity_map, 3, 0, 3, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 0, 4, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 0, 5, forbidden));
  ASSERT_TRUE(nav.exists(registry, arena, entity_map, 3, 0, 50, forbidden));
}
