#include <bm/game/brick_wall.hpp>
#include <bm/game/contest.hpp>
#include <bm/game/player.hpp>
#include <bm/game/player_action.hpp>
#include <bm/game/player_direction.hpp>
#include <bm/game/position_on_grid.hpp>

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

  for(std::uint8_t y = 0; y != arena_height; ++y)
    for(std::uint8_t x = 0; x != arena_width; ++x)
      if(arena.is_static_wall(x, y))
        screen_buffer[y][x] = "\033[100m \033[0;0m";

  registry.view<bm::game::position_on_grid, bm::game::brick_wall>().each(
      [&screen_buffer](const bm::game::position_on_grid& p) -> void
      {
        screen_buffer[p.y][p.x] = "\033[33m#\033[0;0m";
      });

  registry.view<bm::game::player>().each(
      [&screen_buffer](const bm::game::player& p) -> void
      {
        screen_buffer[p.y][p.x] = "\033[32mA\033[0;0m";
      });

  constexpr std::string_view clear_screen = "\x1B[2J";
  constexpr std::string_view move_top_left = "\x1B[H";

  std::cout << clear_screen << move_top_left;

  for(std::uint8_t y = 0; y != arena_height; ++y)
    {
      for(std::uint8_t x = 0; x != arena_width; ++x)
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
        while(!quit.load())
          input.store(std::getchar());
      });

  // Only a single player currently.
  entt::entity local_player = registry.view<bm::game::player>()[0];
  bm::game::player_action& player_action
      = registry.get<bm::game::player_action>(local_player);

  while(!quit.load())
    {
      player_action = bm::game::player_action{};

      switch(input.exchange(0))
        {
        case 'q':
          quit.store(true);
          break;

        case 'A':
          player_action.requested = bm::game::player_direction::up;
          break;
        case 'B':
          player_action.requested = bm::game::player_direction::down;
          break;
        case 'C':
          player_action.requested = bm::game::player_direction::right;
          break;
        case 'D':
          player_action.requested = bm::game::player_direction::left;
          break;
        case ' ':
          player_action.drop_bomb = true;
          break;
        }

      contest.tick();
      display(contest);
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

  if(input_thread.joinable())
    input_thread.join();

  tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);

  return EXIT_SUCCESS;
}
