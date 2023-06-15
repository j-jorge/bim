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
#include <bm/app/console/display.hpp>

#include <bm/game/arena.hpp>
#include <bm/game/component/bomb.hpp>
#include <bm/game/component/brick_wall.hpp>
#include <bm/game/component/flame.hpp>
#include <bm/game/component/flame_direction.hpp>
#include <bm/game/component/player.hpp>
#include <bm/game/component/position_on_grid.hpp>
#include <bm/game/contest.hpp>

#include <entt/entity/registry.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

void bm::app::console::display(const bm::game::contest& contest)
{
  const entt::registry& registry = contest.registry();
  const bm::game::arena& arena = contest.arena();
  const std::uint8_t arena_width = arena.width();
  const std::uint8_t arena_height = arena.height();

  std::vector<std::vector<std::string>> screen_buffer(
      arena_height, std::vector<std::string>(arena_width, " "));

  for (std::uint8_t y = 0; y != arena_height; ++y)
    for (std::uint8_t x = 0; x != arena_width; ++x)
      if (arena.is_static_wall(x, y))
        screen_buffer[y][x] = "\033[100m \033[0;0m";

  registry.view<bm::game::position_on_grid, bm::game::brick_wall>().each(
      [&screen_buffer](const bm::game::position_on_grid& p) -> void
      {
        screen_buffer[p.y][p.x] = "\033[33m#\033[0;0m";
      });

  registry.view<bm::game::position_on_grid, bm::game::bomb>().each(
      [&screen_buffer](const bm::game::position_on_grid& p,
                       const bm::game::bomb& b) -> void
      {
        const size_t f = (b.duration_until_explosion > std::chrono::seconds(1))
                             ? 300
                             : 100;

        if (b.duration_until_explosion.count() / f % 2 == 0)
          screen_buffer[p.y][p.x] = "\033[31mó\033[0;0m";
        else
          screen_buffer[p.y][p.x] = "\033[91mó\033[0;0m";
      });

  registry.view<bm::game::position_on_grid, bm::game::flame>().each(
      [&screen_buffer](const bm::game::position_on_grid& p,
                       const bm::game::flame& f) -> void
      {
        if (f.horizontal == bm::game::flame_horizontal::yes)
          if (f.vertical == bm::game::flame_vertical::yes)
            screen_buffer[p.y][p.x] = "\033[31m+\033[0;0m";
          else
            screen_buffer[p.y][p.x] = "\033[31m-\033[0;0m";
        else
          screen_buffer[p.y][p.x] = "\033[31m|\033[0;0m";
      });

  registry.view<bm::game::player, bm::game::position_on_grid>().each(
      [&screen_buffer](const bm::game::player& player,
                       const bm::game::position_on_grid& p) -> void
      {
        static constexpr char player_character[] = { 'A', 'B', 'C', 'D' };
        screen_buffer[p.y][p.x] = "\033[32m";
        screen_buffer[p.y][p.x] += player_character[player.index];
        screen_buffer[p.y][p.x] += "\033[0;0m";
      });

  constexpr std::string_view clear_screen = "\x1B[2J";
  constexpr std::string_view move_top_left = "\x1B[H";

  std::cout << clear_screen << move_top_left;

  for (std::uint8_t y = 0; y != arena_height; ++y)
    {
      for (std::uint8_t x = 0; x != arena_width; ++x)
        std::cout << screen_buffer[y][x];

      std::cout << '\n';
    }
}
