// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/level_generation.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/factory/crate.hpp>

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

void bim::game::insert_random_crates(arena& arena, entt::registry& registry,
                                     random_generator& random_generator,
                                     std::uint8_t crate_probability,
                                     feature_flags features)
{
  const int width = arena.width();
  const int height = arena.height();

  boost::random::uniform_int_distribution<std::uint8_t> random(0, 99);

  std::vector<position_on_grid> forbidden_positions;
  // Typically 9 blocks for each player: their position and the cells around
  // them.
  forbidden_positions.reserve(g_max_player_count * 9);

  registry.view<bim::game::player, bim::game::fractional_position_on_grid>()
      .each(
          [&forbidden_positions](
              const bim::game::player&,
              const bim::game::fractional_position_on_grid& p) -> void
          {
            const std::uint8_t player_x = p.grid_aligned_x();
            const std::uint8_t player_y = p.grid_aligned_y();

            bim_assume(player_x > 0);
            bim_assume(player_y > 0);

            for (int y : { -1, 0, 1 })
              for (int x : { -1, 0, 1 })
                forbidden_positions.push_back(
                    position_on_grid(player_x + x, player_y + y));
          });

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
      if (!arena.is_static_wall(x, y)
          && !boost::algorithm::any_of_equal(forbidden_positions,
                                             position_on_grid(x, y))
          && (random(random_generator) < crate_probability))
        {
          crates.emplace_back(crate_factory(registry, arena, x, y));

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
