#include <bm/game/contest.hpp>

#include <entt/entity/registry.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
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
  constexpr std::string_view clear_screen = "\x1B[2J";
  constexpr std::string_view move_top_left = "\x1B[H";

  std::cout << clear_screen << move_top_left;

  const bm::game::arena& arena = contest.arena();
  const std::string_view solid_wall
      = g_assets.find(bm::game::item_type::solid_wall)->second;

  for(int x = 0, width = arena.width(); x != width + 2; ++x)
    std::cout << solid_wall;

  std::cout << '\n';

  for(int y = 0, height = arena.height(); y != height; ++y)
    {
      std::cout << solid_wall;

      for(int x = 0, width = arena.width(); x != width; ++x)
        std::cout << ' ';

      std::cout << solid_wall << '\n';
    }

  for(int x = 0, width = arena.width(); x != width + 2; ++x)
    std::cout << solid_wall;

  std::cout << '\n';
}

int main()
{
  termios original_terminal;
  tcgetattr(STDIN_FILENO, &original_terminal);

  termios custom_terminal = original_terminal;
  custom_terminal.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &custom_terminal);

  entt::registry registry;
  bm::game::contest contest(registry, 1, 13, 11);
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
