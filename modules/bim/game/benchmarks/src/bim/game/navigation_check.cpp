// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/level_generation.hpp>

#include <bim/table_2d.impl.hpp>

#include <entt/entity/registry.hpp>

#include <benchmark/benchmark.h>

static void navigation_check_reachable(benchmark::State& state)
{
  const int arena_width = bim::game::g_default_arena_width;
  const int arena_height = bim::game::g_default_arena_height;

  bim::game::arena arena(arena_width, arena_height);
  bim::game::generate_basic_level_structure(arena);
  bim::game::navigation_check nav;

  std::vector<bim::game::position_on_grid> start;
  start.reserve(arena.width() * arena.height());

  for (std::size_t y = 0; y != arena.height(); ++y)
    for (std::size_t x = 0; x != arena.width(); ++x)
      if (!arena.is_static_wall(x, y))
        start.emplace_back(x, y);

  for (auto _ : state)
    for (const bim::game::position_on_grid& p : start)
      {
        bool r = nav.reachable(arena, p.x, p.y, arena_width - p.x - 1,
                               arena_height - p.y - 1);
        benchmark::DoNotOptimize(r);
      }

  state.counters["Calls"] = start.size();
}

BENCHMARK(navigation_check_reachable);

static void navigation_check_paths(benchmark::State& state)
{
  bim::game::arena arena(bim::game::g_default_arena_width,
                         bim::game::g_default_arena_height);
  bim::game::generate_basic_level_structure(arena);
  bim::game::navigation_check nav;
  bim::table_2d<std::uint8_t> distance(arena.width(), arena.height());
  bim::table_2d<bim::game::position_on_grid> previous(arena.width(),
                                                      arena.height());
  entt::registry registry;
  bim::game::entity_world_map entity_map(arena.width(), arena.height());

  std::vector<bim::game::position_on_grid> start;
  start.reserve(arena.width() * arena.height());

  // Add some solid components such that the tests in navigation_check::paths()
  // have some data.
  for (std::size_t i = 0; i != arena.width() * arena.height(); ++i)
    registry.emplace<bim::game::solid>(registry.create());

  for (std::size_t y = 0; y != arena.height(); ++y)
    for (std::size_t x = 0; x != arena.width(); ++x)
      if (!arena.is_static_wall(x, y))
        {
          start.emplace_back(x, y);

          // Make sure we have something in the entity_map such that the tests
          // in navigation_check::paths() have some data.
          entity_map.put_entity(registry.create(), x, y);
        }

  bim::table_2d<bool> allowed(arena.width(), arena.height(), true);

  for (auto _ : state)
    for (const bim::game::position_on_grid& p : start)
      {
        nav.paths(distance, previous, registry, arena, entity_map, p.x, p.y,
                  allowed);
        benchmark::DoNotOptimize(distance);
        benchmark::DoNotOptimize(previous);
      }

  state.counters["Calls"] = start.size();
}

BENCHMARK(navigation_check_paths);

static void navigation_check_exists(benchmark::State& state)
{
  bim::game::arena arena(bim::game::g_default_arena_width,
                         bim::game::g_default_arena_height);
  bim::game::generate_basic_level_structure(arena);
  bim::game::navigation_check nav;
  bim::table_2d<std::uint8_t> distance(arena.width(), arena.height());
  bim::table_2d<bim::game::position_on_grid> previous(arena.width(),
                                                      arena.height());
  entt::registry registry;
  bim::game::entity_world_map entity_map(arena.width(), arena.height());

  std::vector<bim::game::position_on_grid> start;
  start.reserve(arena.width() * arena.height());

  // Add some solid components such that the tests in navigation_check::paths()
  // have some data.
  for (std::size_t i = 0; i != arena.width() * arena.height(); ++i)
    registry.emplace<bim::game::solid>(registry.create());

  for (std::size_t y = 0; y != arena.height(); ++y)
    for (std::size_t x = 0; x != arena.width(); ++x)
      if (!arena.is_static_wall(x, y))
        {
          start.emplace_back(x, y);

          // Make sure we have something in the entity_map such that the tests
          // in navigation_check::paths() have some data.
          entity_map.put_entity(registry.create(), x, y);
        }

  bim::table_2d<bool> forbidden(arena.width(), arena.height(), true);

  for (auto _ : state)
    for (const bim::game::position_on_grid& p : start)
      benchmark::DoNotOptimize(
          nav.exists(registry, arena, entity_map, p.x, p.y, 50, forbidden));

  state.counters["Calls"] = start.size();
}

BENCHMARK(navigation_check_exists);
