// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest.hpp>

#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/constant/default_arena_size.hpp>

#include <entt/entity/registry.hpp>

#include <algorithm>
#include <cstdio>
#include <numeric>

#include <gtest/gtest.h>

TEST(bim_game_contest, power_up_distribution)
{
  constexpr int arena_width = bim::game::g_default_arena_width;
  constexpr int arena_height = bim::game::g_default_arena_height;

  std::array<bool, arena_width * arena_height> usable_cells;
  usable_cells.fill(true);

  constexpr int brick_wall_probability = 80;
  constexpr int player_count = 4;

  // Get teh basic structure of the level: count in usable_cells all the cells
  // that are not occupied by a static wall or the player, nor are located near
  // the players.
  {
    const bim::game::contest contest(0, brick_wall_probability, player_count,
                                     arena_width, arena_height, {});

    for (int y = 0; y != arena_height; ++y)
      for (int x = 0; x != arena_width; ++x)
        if (contest.arena().is_static_wall(x, y))
          usable_cells[y * arena_width + x] = false;

    contest.registry()
        .view<bim::game::fractional_position_on_grid, bim::game::player>()
        .each(
            [&usable_cells](const bim::game::fractional_position_on_grid& p,
                            const bim::game::player&) -> void
            {
              const int x = p.grid_aligned_x();
              const int y = p.grid_aligned_y();

              usable_cells[y * arena_width + x - 1] = false;
              usable_cells[y * arena_width + x] = false;
              usable_cells[y * arena_width + x + 1] = false;
              usable_cells[(y - 1) * arena_width + x] = false;
              usable_cells[y * arena_width + x] = false;
              usable_cells[(y + 1) * arena_width + x] = false;
            });
  }

  std::array<int, arena_width * arena_height> sum_per_cell;
  sum_per_cell.fill(0);

  const auto count_power_up = [&sum_per_cell, &usable_cells](
                                  const bim::game::position_on_grid& p) -> void
  {
    ASSERT_TRUE(usable_cells[p.y * arena_width + p.x])
        << "x=" << (int)p.x << ", y=" << p.y;
    ++sum_per_cell[p.y * arena_width + p.x];
  };

  constexpr int iterations = 1000;
  for (int i = 0; i != iterations; ++i)
    {
      const bim::game::contest contest(i, brick_wall_probability, player_count,
                                       arena_width, arena_height, {});

      contest.registry()
          .view<bim::game::position_on_grid,
                bim::game::flame_power_up_spawner>()
          .each(count_power_up);
      contest.registry()
          .view<bim::game::position_on_grid,
                bim::game::bomb_power_up_spawner>()
          .each(count_power_up);
    }

  const int power_up_count =
      std::accumulate(sum_per_cell.begin(), sum_per_cell.end(), 0);
  const int usable_cell_count = std::ranges::count(usable_cells, true);
  const int expected_count_per_cell = power_up_count / usable_cell_count;
  ASSERT_NE(0, expected_count_per_cell);

  const auto print_failure = [&sum_per_cell](int cx, int cy) -> void
  {
    for (int y = 0; y != arena_height; ++y)
      {
        for (int x = 0; x != arena_width; ++x)
          if ((y == cy) && (x == cx))
            printf(" [%.3d]", sum_per_cell[y * arena_width + x]);
          else
            printf("  %.3d ", sum_per_cell[y * arena_width + x]);

        printf("\n");
      }
  };

  for (int y = 0; y != arena_height; ++y)
    for (int x = 0; x != arena_width; ++x)
      {
        const int i = y * arena_width + x;

        if (!usable_cells[i])
          continue;

        const int delta = std::abs(sum_per_cell[i] - expected_count_per_cell);

        EXPECT_LE(delta * 100 / expected_count_per_cell,
                  10 * expected_count_per_cell / 100)
            << "sum_per_cell[i]=" << sum_per_cell[i]
            << ", expected_count_per_cell=" << expected_count_per_cell
            << ", delta=" << delta << " x=" << x << ", y=" << y;

        if (testing::Test::HasFailure())
          {
            print_failure(x, y);
            return;
          }
      }
}
