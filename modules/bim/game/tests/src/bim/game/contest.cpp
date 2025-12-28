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

static void validate_distribution(const bim::table_2d<int>& sum_per_cell,
                                  const bim::table_2d<bool>& usable_cells)
{
  ASSERT_EQ(sum_per_cell.width(), usable_cells.width());
  ASSERT_EQ(sum_per_cell.height(), usable_cells.height());

  const int width = sum_per_cell.width();
  const int height = sum_per_cell.height();

  const int count =
      std::accumulate(sum_per_cell.begin(), sum_per_cell.end(), 0);
  const int usable_cell_count = std::ranges::count(usable_cells, true);
  const int expected_count_per_cell = count / usable_cell_count;
  ASSERT_NE(0, expected_count_per_cell);

  const auto print_failure = [=, &sum_per_cell](int cx, int cy) -> void
  {
    for (int y = 0; y != height; ++y)
      {
        for (int x = 0; x != width; ++x)
          if ((y == cy) && (x == cx))
            printf(" [%.3d]", sum_per_cell(x, y));
          else
            printf("  %.3d ", sum_per_cell(x, y));

        printf("\n");
      }
  };

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
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

class bim_game_contest_power_up_distribution
  : public testing::TestWithParam<bim::game::feature_flags>
{
protected:
  static constexpr int arena_width = bim::game::g_default_arena_width;
  static constexpr int arena_height = bim::game::g_default_arena_height;

protected:
  template <typename Spawner>
  void run_test(std::size_t iterations, std::size_t expected_count);
  template <typename Spawner>
  void run_test(bim::table_2d<bool> usable_cells_ref, std::size_t iterations,
                std::size_t expected_count);
};

template <typename Spawner>
void bim_game_contest_power_up_distribution::run_test(
    std::size_t iterations, std::size_t expected_count)
{
  bim::table_2d<bool> usable_cells(arena_width, arena_height, true);
  run_test<Spawner>(std::move(usable_cells), iterations, expected_count);
}

template <typename Spawner>
void bim_game_contest_power_up_distribution::run_test(
    bim::table_2d<bool> usable_cells_ref, std::size_t iterations,
    std::size_t expected_count)
{
  ASSERT_EQ(arena_width, usable_cells_ref.width());
  ASSERT_EQ(arena_height, usable_cells_ref.height());

  const bim::game::feature_flags feature_flags = GetParam();
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

  // Get the basic structure of the level: count in usable_cells all the
  // cells that are not occupied by a static wall or the player, nor are
  // located near the players.
  {
    const bim::game::contest contest(fingerprint);
    const bim::game::arena& arena = contest.arena();

    for (int y = 0; y != arena_height; ++y)
      for (int x = 0; x != arena_width; ++x)
        if (arena.is_static_wall(x, y))
          usable_cells_ref(x, y) = false;

    contest.registry()
        .view<bim::game::fractional_position_on_grid, bim::game::player>()
        .each(
            [&usable_cells_ref](
                const bim::game::fractional_position_on_grid& p,
                const bim::game::player&) -> void
            {
              const int x = p.grid_aligned_x();
              const int y = p.grid_aligned_y();

              usable_cells_ref(x - 1, y - 1) = false;
              usable_cells_ref(x, y - 1) = false;
              usable_cells_ref(x + 1, y - 1) = false;

              usable_cells_ref(x - 1, y) = false;
              usable_cells_ref(x, y) = false;
              usable_cells_ref(x + 1, y) = false;

              usable_cells_ref(x - 1, y + 1) = false;
              usable_cells_ref(x, y + 1) = false;
              usable_cells_ref(x + 1, y + 1) = false;
            });
  }

  bim::table_2d<int> sum_per_cell(arena_width, arena_height, 0);

  for (std::size_t i = 0; i != iterations; ++i)
    {
      fingerprint.seed = i;
      const bim::game::contest contest(fingerprint);
      bim::table_2d<bool> usable_cells(usable_cells_ref);
      const bim::game::arena& arena = contest.arena();

      // Remove the randomly-placed fences from the usable cells of this
      // iteration.
      for (int y = 0; y != arena_height; ++y)
        for (int x = 0; x != arena_width; ++x)
          if (!!arena.fences(x, y))
            usable_cells(x, y) = false;

      std::size_t count = 0;

      for (const auto&& [_, p] :
           contest.registry()
               .view<bim::game::position_on_grid, Spawner>()
               .each())
        {
          ASSERT_TRUE(usable_cells(p.x, p.y))
              << "x=" << (int)p.x << ", y=" << (int)p.y;
          ++sum_per_cell(p.x, p.y);
          ++count;
        }

      ASSERT_EQ(expected_count, count);
    }

  validate_distribution(sum_per_cell, usable_cells_ref);
}

TEST_P(bim_game_contest_power_up_distribution, bomb_power_up)
{
  run_test<bim::game::bomb_power_up_spawner>(
      3000, bim::game::g_bomb_power_up_count_in_level);
}

TEST_P(bim_game_contest_power_up_distribution, flame_power_up)
{
  run_test<bim::game::flame_power_up_spawner>(
      4000, bim::game::g_flame_power_up_count_in_level);
}

TEST_P(bim_game_contest_power_up_distribution, invisibility_power_up)
{
  if (!(GetParam() & bim::game::feature_flags::invisibility))
    return;

  bim::table_2d<bool> usable_cells(arena_width, arena_height);

  for (int y = 0; y != arena_height; ++y)
    for (int x = 0; x != arena_width; ++x)
      usable_cells(x, y) = bim::game::valid_invisibility_power_up_position(
          x, y, arena_width, arena_height);

  run_test<bim::game::invisibility_power_up_spawner>(
      usable_cells, 6000, bim::game::g_invisibility_power_up_count_in_level);
}

TEST_P(bim_game_contest_power_up_distribution, shield_power_up)
{
  if (!(GetParam() & bim::game::feature_flags::shield))
    return;

  run_test<bim::game::shield_power_up_spawner>(
      6000, bim::game::g_shield_power_up_count_in_level);
}

INSTANTIATE_TEST_SUITE_P(
    bim_game_contest_power_up_distribution_suite,
    bim_game_contest_power_up_distribution,
    testing::Values(bim::game::feature_flags::invisibility,
                    bim::game::feature_flags::shield,
                    bim::game::feature_flags::invisibility
                        | bim::game::feature_flags::shield,
                    bim::game::feature_flags::fences),
    [](const testing::TestParamInfo<
        bim_game_contest_power_up_distribution::ParamType>& info)
    {
      std::string result;
      const char* separator = "";

      if (!!(info.param & bim::game::feature_flags::invisibility))
        {
          result += "invisibility";
          separator = "_";
        }
      if (!!(info.param & bim::game::feature_flags::shield))
        {
          result += separator;
          result += "shield";
          separator = "_";
        }
      if (!!(info.param & bim::game::feature_flags::fences))
        {
          result += separator;
          result += "fences";
        }
      return result;
    });
