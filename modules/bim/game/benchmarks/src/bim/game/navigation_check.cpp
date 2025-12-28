// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/level_generation.hpp>

#include <benchmark/benchmark.h>

static void navigation_check(benchmark::State& state)
{
  bim::game::arena arena(bim::game::g_default_arena_width,
                         bim::game::g_default_arena_height);
  bim::game::generate_basic_level_structure(arena);
  bim::game::navigation_check nav;

  for (auto _ : state)
    for (int y0 = 0; y0 != bim::game::g_default_arena_height; ++y0)
      for (int x0 = 0; x0 != bim::game::g_default_arena_width; ++x0)
        if (!arena.is_static_wall(x0, y0))
          for (int y1 = 0; y1 != bim::game::g_default_arena_height; ++y1)
            for (int x1 = 0; x1 != bim::game::g_default_arena_width; ++x1)
              if (!arena.is_static_wall(x1, y1))
                {
                  bool r = nav.reachable(arena, x0, y0, x1, y1);
                  benchmark::DoNotOptimize(r);
                }
}

BENCHMARK(navigation_check);
