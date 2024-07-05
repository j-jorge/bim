/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bim/game/level_generation.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/factory/brick_wall.hpp>

#include <bim/game/random_generator.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/random/uniform_int_distribution.hpp>

void bim::game::generate_basic_level_structure(arena& arena)
{
  const int width = arena.width();
  const int height = arena.height();

  bim_assume(width >= 3);
  bim_assume(height >= 3);

  // Unbreakable walls on the borders.
  for (int x = 0; x != width; ++x)
    arena.set_static_wall(x, 0);

  for (int x = 0; x != width; ++x)
    arena.set_static_wall(x, height - 1);

  for (int y = 1; y < height - 1; ++y)
    arena.set_static_wall(0, y);

  for (int y = 1; y < height - 1; ++y)
    arena.set_static_wall(width - 1, y);

  // Unbreakable walls in the game area.
  for (int y = 2; y < height - 1; y += 2)
    for (int x = 2; x < width - 1; x += 2)
      arena.set_static_wall(x, y);
}

void bim::game::insert_random_brick_walls(arena& arena,
                                          entt::registry& registry,
                                          random_generator& random_generator,
                                          std::uint8_t brick_wall_probability)
{
  const int width = arena.width();
  const int height = arena.height();

  boost::random::uniform_int_distribution<std::uint8_t> random(0, 99);

  std::vector<position_on_grid> forbidden_positions;
  // Typically 9 blocks for each player: their position and the cells around
  // them..
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow="
                // This warning is emitted by GCC 13.2.0 on this line, yet
                // I see no issue in this code.
                forbidden_positions.push_back(
                    position_on_grid(player_x + x, player_y + y));
#pragma GCC diagnostic pop
          });

  std::vector<entt::entity> brick_walls;
  brick_walls.reserve(width * height);

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      if (!arena.is_static_wall(x, y)
          && !boost::algorithm::any_of_equal(forbidden_positions,
                                             position_on_grid(x, y))
          && (random(random_generator) < brick_wall_probability))
        brick_walls.emplace_back(brick_wall_factory(registry, arena, x, y));

  // Select the walls that will spawn power-ups.
  const std::size_t brick_wall_count = brick_walls.size();
  const std::size_t power_up_count = std::min<std::size_t>(
      brick_wall_count,
      g_bomb_power_up_count_in_level + g_flame_power_up_count_in_level);

  for (std::size_t i = 0, n = power_up_count; i != n; ++i)
    {
      const std::size_t j =
          i + random(random_generator) % (brick_wall_count - i);
      std::swap(brick_walls[i], brick_walls[j]);
    }

  std::size_t i = 0;

  // The bomb power-ups.
  for (std::size_t j = 0;
       (j != g_bomb_power_up_count_in_level) && (i != brick_wall_count);
       ++i, ++j)
    registry.emplace<bomb_power_up_spawner>(brick_walls[i]);

  // The flame power-ups.
  for (std::size_t j = 0;
       (j != g_flame_power_up_count_in_level) && (i != brick_wall_count);
       ++i, ++j)
    registry.emplace<flame_power_up_spawner>(brick_walls[i]);
}
