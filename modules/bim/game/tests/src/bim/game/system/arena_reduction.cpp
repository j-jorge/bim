// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/arena_reduction.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/cell_edge.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/factory/arena_reduction.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

static void check_positions(
    entt::registry& registry, bim::game::arena& arena,
    std::span<const bim::game::position_on_grid> expected_positions)
{
  constexpr std::chrono::milliseconds start_delay(10);
  constexpr std::chrono::milliseconds tick_duration(1);

  const entt::entity e =
      bim::game::arena_reduction_factory(registry, start_delay);
  const bim::game::timer& t = registry.storage<bim::game::timer>().get(e);
  bim::game::arena_reduction reduction(arena);

  for (bool stop = false; !stop;)
    {
      bim::game::update_timers(registry, tick_duration);

      stop = (t.duration.count() == 0);
      reduction.update(registry, arena);
    }

  for (std::size_t i = 0; i != expected_positions.size(); ++i)
    {
      int falling_block_count = 0;

      registry.view<bim::game::falling_block, bim::game::position_on_grid>()
          .each(
              [&](bim::game::position_on_grid p) -> void
              {
                if (p != expected_positions[i])
                  return;

                ++falling_block_count;
                EXPECT_EQ(1, falling_block_count)
                    << "i=" << i << ", p=(" << (int)p.x << ", " << (int)p.y
                    << ')';
              });

      EXPECT_EQ(1, falling_block_count) << "i=" << i << ", expected_position=("
                                        << (int)expected_positions[i].x << ", "
                                        << (int)expected_positions[i].y << ')';
      // The timer should have been reset for the next block.
      EXPECT_NE(0, t.duration.count());

      for (bool stop = false; !stop;)
        {
          bim::game::update_timers(registry, tick_duration);

          stop = (t.duration.count() == 0);
          reduction.update(registry, arena);
        }
    }
}

TEST(bim_game_arena_reduction, update)
{
  entt::registry registry;
  bim::game::arena arena(3, 4);
  arena.set_static_wall(0, 0, bim::game::cell_neighborhood::none);
  arena.set_static_wall(0, 2, bim::game::cell_neighborhood::none);

  /*
    x01
    684
    x97
    352
  */
  const bim::game::position_on_grid expected_positions[] = {
    { 1, 0 }, // horizontal
    { 2, 0 }, // vertical
    { 2, 3 }, // horizontal backward
    { 0, 3 }, // vertical backward
    // horizontal finds nothing
    { 2, 1 }, // vertical
    { 1, 3 }, // horizontal backward
    { 0, 1 }, // vertical backward
    // horizontal is still on first row and finds nothing
    { 2, 2 }, // vertical
    // horizontal backward finds nothing
    // vertical backward finds nothing
    { 1, 1 },
    { 1, 2 }
  };
  constexpr std::size_t expected_positions_size =
      std::size(expected_positions);
  ASSERT_EQ(arena.width() * arena.height() - 2, expected_positions_size);

  check_positions(registry, arena, expected_positions);
}

TEST(bim_game_arena_reduction, fall_on_fences)
{
  entt::registry registry;
  bim::game::arena arena(3, 4);
  arena.add_fence(0, 0, bim::game::cell_edge::down);
  arena.add_fence(0, 2, bim::game::cell_edge::right);
  arena.add_fence(1, 1, bim::game::cell_edge::left);
  arena.add_fence(2, 3, bim::game::cell_edge::up);

  /*
    041
    9a5
    7b8
    362
  */
  const bim::game::position_on_grid expected_positions[] = {
    { 0, 0 }, { 2, 0 }, { 2, 3 }, { 0, 3 }, { 1, 0 }, { 2, 1 },
    { 1, 3 }, { 0, 2 }, { 2, 2 }, { 0, 1 }, { 1, 1 }, { 1, 2 }
  };
  constexpr std::size_t expected_positions_size =
      std::size(expected_positions);
  ASSERT_EQ(arena.width() * arena.height(), expected_positions_size);

  check_positions(registry, arena, expected_positions);
}
