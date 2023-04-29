#include <bm/game/brick_wall.hpp>
#include <bm/game/contest.hpp>
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

namespace bm
{
  namespace game
  {
    enum class item_type : uint8_t
    {
      solid_wall,
      player
    };
  }
}

static const std::unordered_map<bm::game::item_type, std::string_view> g_assets
    = { { bm::game::item_type::solid_wall, "\033[90;0m░" },
        { bm::game::item_type::player, "\033[90;0m🯅" } };

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
  bool special = false;

  std::atomic<bool> quit(false);
  std::atomic<int> input(0);

  std::thread input_thread(
      [&input, &quit]() -> void
      {
        while(!quit.load())
          input.store(std::getchar());
      });

  while(!quit.load())
    {
      switch(input.load())
        {
        case 'q':
          quit.store(true);
          break;

        case 'A':
          if(special)
            {}
          // up
          break;
        case 'B':
          if(special)
            {}
          // down
          break;
        case 'C':
          if(special)
            {}
          // right
          break;
        case 'D':
          if(special)
            {}
          // left
          break;
        case ' ':
          // bomb
          break;

        case 91:
          special = true;
          break;
        }

      special = false;
      contest.tick();
      display(contest);
      std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

  if(input_thread.joinable())
    input_thread.join();

  tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);

  return EXIT_SUCCESS;
}
