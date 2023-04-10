#include <bm/game/contest.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

#include <termios.h>
#include <unistd.h>

int main()
{
  termios terminal;
  tcgetattr(STDIN_FILENO, &terminal);
  terminal.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);

  bm::game::contest contest(1, 13, 11);
  bool special = false;
  bool quit = false;

  for (int c; !quit && ((c = std::getchar()) != EOF); )
    {
      switch (c)
        {
        case 27:
        case 'q':
          quit = true;
          break;

        case 'A':
          if (special) {}
            // up
          break;
        case 'B':
          if (special) {}
            // down
          break;
        case 'C':
          if (special) {}
            // right
          break;
        case 'D':
          if (special) {}
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
      std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

  return EXIT_SUCCESS;
}
