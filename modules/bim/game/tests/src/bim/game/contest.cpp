// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/feature_flags.hpp>
#include <bim/game/level_generation.hpp>

#include <bim/table_2d.impl.hpp>

#include <entt/entity/registry.hpp>

#include <algorithm>
#include <cstdio>
#include <numeric>

#include <gtest/gtest.h>

class bim_game_contest_power_up_distribution : public testing::Test
{
protected:
  static constexpr int arena_width = bim::game::g_default_arena_width;
  static constexpr int arena_height = bim::game::g_default_arena_height;

protected:
  template <typename Spawner>
  void run_test(bim::game::feature_flags feature_flags, std::size_t iterations,
                std::size_t expected_count);
  template <typename Spawner>
  void run_test(bim::table_2d<bool>& usable_cells,
                bim::game::feature_flags feature_flags, std::size_t iterations,
                std::size_t expected_count);
};

template <typename Spawner>
void bim_game_contest_power_up_distribution::run_test(
    bim::game::feature_flags feature_flags, std::size_t iterations,
    std::size_t expected_count)
{
  bim::table_2d<bool> usable_cells(arena_width, arena_height, true);
  run_test<Spawner>(usable_cells, feature_flags, iterations, expected_count);
}

template <typename Spawner>
void bim_game_contest_power_up_distribution::run_test(
    bim::table_2d<bool>& usable_cells, bim::game::feature_flags feature_flags,
    std::size_t iterations, std::size_t expected_count)
{
  ASSERT_EQ(arena_width, usable_cells.width());
  ASSERT_EQ(arena_height, usable_cells.height());

  constexpr int crate_probability = 80;
  constexpr int player_count = 4;

  bim::game::contest_fingerprint fingerprint = { .seed = 0,
                                                 .features = feature_flags,
                                                 .player_count = player_count,
                                                 .crate_probability =
                                                     crate_probability,
                                                 .arena_width = arena_width,
                                                 .arena_height =
                                                     arena_height };

  // Get the basic structure of the level: count in usable_cells all the cells
  // that are not occupied by a static wall or the player, nor are located near
  // the players.
  {
    const bim::game::contest contest(fingerprint);

    for (int y = 0; y != arena_height; ++y)
      for (int x = 0; x != arena_width; ++x)
        if (contest.arena().is_static_wall(x, y))
          usable_cells(x, y) = false;

    contest.registry()
        .view<bim::game::fractional_position_on_grid, bim::game::player>()
        .each(
            [&usable_cells](const bim::game::fractional_position_on_grid& p,
                            const bim::game::player&) -> void
            {
              const int x = p.grid_aligned_x();
              const int y = p.grid_aligned_y();

              usable_cells(x - 1, y) = false;
              usable_cells(x, y) = false;
              usable_cells(x + 1, y) = false;
              usable_cells(x, y - 1) = false;
              usable_cells(x, y) = false;
              usable_cells(x, y + 1) = false;
            });
  }

  bim::table_2d<int> sum_per_cell(arena_width, arena_height, 0);

  for (std::size_t i = 0; i != iterations; ++i)
    {
      fingerprint.seed = i;
      const bim::game::contest contest(fingerprint);
      std::size_t count = 0;

      for (const auto&& [_, p] :
           contest.registry()
               .view<bim::game::position_on_grid, Spawner>()
               .each())
        {
          ASSERT_TRUE(usable_cells(p.x, p.y))
              << "x=" << (int)p.x << ", y=" << p.y;
          ++sum_per_cell(p.x, p.y);
          ++count;
        }

      ASSERT_EQ(expected_count, count);
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
            printf(" [%.3d]", sum_per_cell(x, y));
          else
            printf("  %.3d ", sum_per_cell(x, y));

        printf("\n");
      }
  };

  for (int y = 0; y != arena_height; ++y)
    for (int x = 0; x != arena_width; ++x)
      {
        if (!usable_cells(x, y))
          continue;

        const int delta =
            std::abs(sum_per_cell(x, y) - expected_count_per_cell);

        EXPECT_LE(delta * 100 / expected_count_per_cell,
                  10 * expected_count_per_cell / 100)
            << "sum_per_cell(" << x << ", " << y << ")=" << sum_per_cell(x, y)
            << ", expected_count_per_cell=" << expected_count_per_cell
            << ", delta=" << delta;

        if (testing::Test::HasFailure())
          {
            print_failure(x, y);
            return;
          }
      }
}

TEST_F(bim_game_contest_power_up_distribution, bomb_power_up)
{
  run_test<bim::game::bomb_power_up_spawner>(
      {}, 3000, bim::game::g_bomb_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution, bomb_power_up_with_invisibility)
{
  run_test<bim::game::bomb_power_up_spawner>(
      bim::game::feature_flags::invisibility, 3000,
      bim::game::g_bomb_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution, bomb_power_up_with_shield)
{
  run_test<bim::game::bomb_power_up_spawner>(
      bim::game::feature_flags::shield, 3000,
      bim::game::g_bomb_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution,
       bomb_power_up_with_shield_and_invisibility)
{
  run_test<bim::game::bomb_power_up_spawner>(
      bim::game::feature_flags::shield
          | bim::game::feature_flags::invisibility,
      3000, bim::game::g_bomb_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution, flame_power_up)
{
  run_test<bim::game::flame_power_up_spawner>(
      {}, 3000, bim::game::g_flame_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution,
       flame_power_up_with_invisibility)
{
  run_test<bim::game::flame_power_up_spawner>(
      bim::game::feature_flags::invisibility, 4000,
      bim::game::g_flame_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution, flame_power_up_with_shield)
{
  run_test<bim::game::flame_power_up_spawner>(
      bim::game::feature_flags::shield, 4000,
      bim::game::g_flame_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution,
       flame_power_up_with_shield_and_invisibility)
{
  run_test<bim::game::flame_power_up_spawner>(
      bim::game::feature_flags::shield
          | bim::game::feature_flags::invisibility,
      4000, bim::game::g_flame_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution, invisibility_power_up)
{
  bim::table_2d<bool> usable_cells(arena_width, arena_height);

  for (int y = 0; y != arena_height; ++y)
    for (int x = 0; x != arena_width; ++x)
      usable_cells(x, y) = bim::game::valid_invisibility_power_up_position(
          x, y, arena_width, arena_height);

  run_test<bim::game::invisibility_power_up_spawner>(
      usable_cells, bim::game::feature_flags::invisibility, 6000,
      bim::game::g_invisibility_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution,
       invisibility_power_up_with_shield)
{
  bim::table_2d<bool> usable_cells(arena_width, arena_height);

  for (int y = 0; y != arena_height; ++y)
    for (int x = 0; x != arena_width; ++x)
      usable_cells(x, y) = bim::game::valid_invisibility_power_up_position(
          x, y, arena_width, arena_height);

  run_test<bim::game::invisibility_power_up_spawner>(
      usable_cells,
      bim::game::feature_flags::invisibility
          | bim::game::feature_flags::shield,
      6000, bim::game::g_invisibility_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution, shield_power_up)
{
  run_test<bim::game::shield_power_up_spawner>(
      bim::game::feature_flags::shield, 6000,
      bim::game::g_shield_power_up_count_in_level);
}

TEST_F(bim_game_contest_power_up_distribution,
       shield_power_up_and_invisibility)
{
  run_test<bim::game::shield_power_up_spawner>(
      bim::game::feature_flags::shield
          | bim::game::feature_flags::invisibility,
      6000, bim::game::g_shield_power_up_count_in_level);
}
