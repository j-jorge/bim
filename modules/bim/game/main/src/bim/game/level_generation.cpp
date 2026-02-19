// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/level_generation.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>
#include <bim/game/constant/fence_count_ratio.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/factory/crate.hpp>
#include <bim/game/navigation_check.hpp>

#include <bim/game/random_generator.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/random/uniform_int_distribution.hpp>

template <typename Spawner, typename Predicate>
static void generate_power_up_spawners(
    entt::registry& registry, std::vector<entt::entity>& population,
    std::size_t& population_size, std::size_t available, std::size_t needed,
    bim::game::random_generator& random_generator, Predicate&& entity_filter)
{
  if (population_size < needed)
    needed = population_size;

  for (std::size_t i = 0; needed != 0; ++i)
    {
      if ((population[i] == entt::null) || !entity_filter(population[i]))
        continue;

      boost::random::uniform_int_distribution<std::uint8_t> random(1,
                                                                   available);

      if (random(random_generator) <= needed)
        {
          registry.emplace<Spawner>(population[i]);
          population[i] = entt::null;
          --population_size;
          --needed;
        }

      --available;
    }
}

template <typename Spawner>
static void
generate_power_up_spawners(entt::registry& registry,
                           std::vector<entt::entity>& population,
                           std::size_t& population_size, std::size_t needed,
                           bim::game::random_generator& random_generator)
{
  generate_power_up_spawners<Spawner>(registry, population, population_size,
                                      population_size, needed,
                                      random_generator,
                                      [](entt::entity) -> bool
                                      {
                                        return true;
                                      });
}

void bim::game::generate_basic_level_structure(arena& arena)
{
  const int width = arena.width();
  const int height = arena.height();

  bim_assume(width >= 3);
  bim_assume(height >= 3);

  // Unbreakable walls in the corners.
  arena.set_static_wall(
      0, 0, cell_neighborhood::all & ~cell_neighborhood::down_right);
  arena.set_static_wall(
      width - 1, 0, cell_neighborhood::all & ~cell_neighborhood::down_left);
  arena.set_static_wall(0, height - 1,
                        cell_neighborhood::all & ~cell_neighborhood::up_right);
  arena.set_static_wall(width - 1, height - 1,
                        cell_neighborhood::all & ~cell_neighborhood::up_left);

  // Unbreakable walls on the borders.
  for (int x = 1; x != width - 1; ++x)
    arena.set_static_wall(x, 0,
                          cell_neighborhood::all & ~cell_neighborhood::down);

  for (int x = 1; x != width - 1; ++x)
    arena.set_static_wall(x, height - 1,
                          cell_neighborhood::all & ~cell_neighborhood::up);

  for (int y = 1; y < height - 1; ++y)
    arena.set_static_wall(0, y,
                          cell_neighborhood::all & ~cell_neighborhood::right);

  for (int y = 1; y < height - 1; ++y)
    arena.set_static_wall(width - 1, y,
                          cell_neighborhood::all & ~cell_neighborhood::left);

  // Unbreakable walls in the game area.
  for (int y = 2; y < height - 1; y += 2)
    for (int x = 2; x < width - 1; x += 2)
      arena.set_static_wall(x, y, cell_neighborhood::none);
}

void bim::game::insert_random_crates(
    const context& context, arena& arena, entity_world_map& entity_map,
    entt::registry& registry, random_generator& random_generator,
    std::uint8_t crate_probability, feature_flags features,
    std::span<const position_on_grid> forbidden_positions)
{
  const int width = arena.width();
  const int height = arena.height();

  boost::random::uniform_int_distribution<std::uint8_t> random(0, 99);

  // Generate the crates and remember their entities such that we can assign
  // them the power-up spawners after.
  std::vector<entt::entity> crates;
  crates.reserve(width * height);

  // The invisibility power-up applies to a restricted set of positions; we
  // keep count of the walls matching these position to parameterize the draw
  // of walls for them.
  std::size_t count_for_invisibility = 0;

  // The actual generation of crates.
  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      if (!arena.is_static_wall(x, y) && !arena.fences(x, y)
          && !boost::algorithm::any_of_equal(forbidden_positions.begin(),
                                             forbidden_positions.end(),
                                             position_on_grid(x, y))
          && (random(random_generator) < crate_probability))
        {
          crates.emplace_back(
              crate_factory(context, registry, entity_map, x, y));

          if (valid_invisibility_power_up_position(x, y, width, height))
            ++count_for_invisibility;
        }

  // Shuffle the walls before assigning the power-ups.
  for (std::size_t i = 0, n = crates.size(); i != n - 1; ++i)
    {
      boost::random::uniform_int_distribution<std::uint8_t> random(i, n - 1);
      std::swap(crates[i], crates[random(random_generator)]);
    }

  // Select the walls that will spawn power-ups.
  std::size_t crate_count = crates.size();

  // The invisibility power-ups.
  if (!!(features & feature_flags::invisibility))
    generate_power_up_spawners<invisibility_power_up_spawner>(
        registry, crates, crate_count, count_for_invisibility,
        g_invisibility_power_up_count_in_level, random_generator,
        [=, &registry](entt::entity e) -> bool
        {
          const position_on_grid p =
              registry.storage<position_on_grid>().get(e);

          return valid_invisibility_power_up_position(p.x, p.y, width, height);
        });

  // The shield power-ups.
  if (!!(features & feature_flags::shield))
    generate_power_up_spawners<shield_power_up_spawner>(
        registry, crates, crate_count, g_shield_power_up_count_in_level,
        random_generator);

  // The bomb power-ups.
  generate_power_up_spawners<bomb_power_up_spawner>(
      registry, crates, crate_count, g_bomb_power_up_count_in_level,
      random_generator);

  // The flame power-ups.
  generate_power_up_spawners<flame_power_up_spawner>(
      registry, crates, crate_count, g_flame_power_up_count_in_level,
      random_generator);
}

bool bim::game::valid_invisibility_power_up_position(int x, int y, int w,
                                                     int h)
{
  bim_assume(x >= 0);
  bim_assume(y >= 0);
  bim_assume(w >= 0);
  bim_assume(h >= 0);
  bim_assume(x < w);
  bim_assume(y < h);

  const int r = (std::min(w, h) + 1) / 2;

  // No invisibility power-up in the corners. Only the dots are valid
  // positions:
  //
  // xxxxxxx
  // xxx.xxx
  // xx...xx
  // x.....x
  // .......
  // x.....x
  // xx...xx
  // xxx.xxx
  // xxxxxxx

  return (x + y > r) && ((w - x - 1) + y > r) && (x + (h - y - 1) > r)
         && ((w - x - 1) + (h - y - 1) > r);
}

void bim::game::insert_random_fences(
    arena& arena, random_generator& random_generator,
    std::span<const position_on_grid> forbidden_positions)
{
  // We are going to generate fences one one quarter of the arena, then we will
  // duplicate the result with symmetry for the other quarters.

  const std::size_t arena_width = arena.width();
  const std::size_t arena_height = arena.height();

  bim_assume(arena_width >= 3);
  bim_assume(arena_height >= 3);

  const std::size_t pattern_width = (arena_width + 1) / 2;
  const std::size_t pattern_height = (arena_height + 1) / 2;

  // We collect the cell positions where we can put a fence. We will draw
  // randomly from this set later on.
  struct candidate
  {
    uint8_t x;
    uint8_t y;
    bool vertical_edge;
  };
  std::vector<candidate> candidates;
  candidates.reserve(pattern_width * pattern_height);

  for (std::size_t y = 1; y < pattern_height; ++y)
    for (std::size_t x = 1; x < pattern_width; ++x)
      {
        if (arena.is_static_wall(x, y)
            || boost::algorithm::any_of_equal(forbidden_positions.begin(),
                                              forbidden_positions.end(),
                                              position_on_grid(x, y)))
          continue;

        const int walls_vertical =
            arena.is_static_wall(x, y - 1) + arena.is_static_wall(x, y + 1);
        const int walls_horizontal =
            arena.is_static_wall(x - 1, y) + arena.is_static_wall(x + 1, y);

        const bool vertical_candidate = walls_vertical == 2;
        const bool horizontal_candidate = walls_horizontal == 2;

        if (vertical_candidate == horizontal_candidate)
          continue;

        // The pattern goes to the middle of the arena, on both dimensions. If
        // we put a fence here then it would create a closed cell when the
        // symmetric fence will be added later in the function. Consequently we
        // avoid fences on the edges.
        const bool allowed = vertical_candidate ? (x + 1 < pattern_width)
                                                : (y + 1 < pattern_height);

        if (!allowed)
          continue;

        candidates.emplace_back(x, y, vertical_candidate);
      }

  navigation_check nav_check;

  const int min_allowed = 2;
  int available = candidates.size();
  boost::random::uniform_int_distribution d(
      min_allowed, std::max(min_allowed, available / g_fence_count_ratio));
  int needed = d(random_generator);

  boost::random::uniform_int_distribution side(0, 1);

  for (const candidate& c : candidates)
    {
      if ((available == 0) || (needed == 0))
        break;

      if (boost::random::uniform_int_distribution(1,
                                                  available)(random_generator)
          > needed)
        {
          --available;
          continue;
        }

      --needed;
      --available;

      const int s = side(random_generator);
      cell_edge e[4];

      if (c.vertical_edge)
        e[0] = s ? cell_edge::left : cell_edge::right;
      else
        e[0] = s ? cell_edge::up : cell_edge::down;

      e[1] = horizontal_flip(e[0]);
      e[2] = vertical_flip(e[0]);
      e[3] = horizontal_flip(e[2]);

      const position_on_grid p[] = {
        position_on_grid(c.x, c.y),
        position_on_grid(arena_width - c.x - 1, c.y),
        position_on_grid(c.x, arena_height - c.y - 1),
        position_on_grid(arena_width - c.x - 1, arena_height - c.y - 1)
      };

      for (int i = 0; i != 4; ++i)
        {
          const int x = p[i].x;
          const int y = p[i].y;

          if (arena.is_static_wall(x, y))
            continue;

          arena.add_fence(x, y, e[i]);

          bool good;

          switch (e[i])
            {
            case cell_edge::left:
              good = nav_check.reachable(arena, x, y, x - 1, y);
              break;
            case cell_edge::right:
              good = nav_check.reachable(arena, x, y, x + 1, y);
              break;
            case cell_edge::up:
              good = nav_check.reachable(arena, x, y, x, y - 1);
              break;
            default:
              assert(e[i] == cell_edge::down);
              good = nav_check.reachable(arena, x, y, x, y + 1);
            }

          if (!good)
            arena.remove_fence(x, y, e[i]);
        }
    }
}
