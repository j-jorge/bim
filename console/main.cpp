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
#include <bm/game/contest.hpp>

#include <bm/game/component/bomb.hpp>
#include <bm/game/component/brick_wall.hpp>
#include <bm/game/component/player.hpp>
#include <bm/game/component/player_action.hpp>
#include <bm/game/component/player_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include <termios.h>
#include <unistd.h>

static void display(const bm::game::contest& contest)
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
        screen_buffer[p.y][p.x] = "\033[31mó\033[0;0m";
      });

  registry.view<bm::game::player, bm::game::position_on_grid>().each(
      [&screen_buffer](const bm::game::player&,
                       const bm::game::position_on_grid& p) -> void
      {
        screen_buffer[p.y][p.x] = "\033[32mA\033[0;0m";
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

int main()
{
  termios original_terminal;
  tcgetattr(STDIN_FILENO, &original_terminal);

  termios custom_terminal = original_terminal;
  custom_terminal.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &custom_terminal);

  bm::game::contest contest(1234, 80, 1, 13, 11);
  entt::registry& registry = contest.registry();

  std::atomic<bool> quit(false);
  std::atomic<int> input(0);

  std::thread input_thread(
      [&input, &quit]() -> void
      {
        while (!quit.load())
          input.store(std::getchar());
      });

  // Only a single player currently.
  entt::entity local_player = registry.view<bm::game::player>()[0];
  bm::game::player_action& player_action
      = registry.get<bm::game::player_action>(local_player);

  // 60 updates per second.
  constexpr std::chrono::duration<std::size_t, std::ratio<1, 60>>
      update_interval(1);

  while (!quit.load())
    {
      const std::chrono::steady_clock::time_point now
          = std::chrono::steady_clock::now();

      player_action = bm::game::player_action{};

      switch (input.exchange(0))
        {
        case 'q':
          quit.store(true);
          break;

        case 'A':
          player_action.requested_direction = bm::game::player_direction::up;
          break;
        case 'B':
          player_action.requested_direction = bm::game::player_direction::down;
          break;
        case 'C':
          player_action.requested_direction
              = bm::game::player_direction::right;
          break;
        case 'D':
          player_action.requested_direction = bm::game::player_direction::left;
          break;
        case ' ':
          player_action.drop_bomb = true;
          break;
        }

      contest.tick(std::chrono::duration_cast<std::chrono::nanoseconds>(
          update_interval));
      display(contest);

      std::this_thread::sleep_until(now + update_interval);
    }

  if (input_thread.joinable())
    input_thread.join();

  tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);

  return EXIT_SUCCESS;
}
