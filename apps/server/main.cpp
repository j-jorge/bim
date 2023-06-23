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
#include <bim/server/server.hpp>

#include <iscool/log/enable_console_log.h>
#include <iscool/log/setup.h>
#include <iscool/schedule/manual_scheduler.h>
#include <iscool/schedule/setup.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

bool g_keep_running = true;

static void interrupt_handler(int)
{
  g_keep_running = false;
}

static void install_signal_handlers()
{
  struct sigaction action;

  action.sa_handler = interrupt_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  sigaction(SIGINT, &action, nullptr);
}

int main(int argc, char* argv[])
{
  install_signal_handlers();

  iscool::log::scoped_initializer log;
  iscool::log::enable_console_log();

  iscool::schedule::manual_scheduler scheduler;
  iscool::schedule::initialize(scheduler.get_delayed_call_delegate());

  constexpr unsigned short port = 23899;

  std::cout << "Running on port " << port << '\n'
            << "Press Ctrl+C to exit." << '\n';

  bim::server::server server(port);

  using clock = std::chrono::steady_clock;

  std::chrono::nanoseconds slice_duration(0);
  clock::time_point last_update = clock::now();

  constexpr std::chrono::milliseconds tick_interval(10);

  while (g_keep_running)
    {
      const clock::time_point start = clock::now();
      slice_duration += start - last_update;
      last_update = start;

      if (slice_duration >= std::chrono::milliseconds(1))
        {
          const std::chrono::milliseconds update_ms =
              std::chrono::duration_cast<std::chrono::milliseconds>(
                  slice_duration);

          slice_duration -= update_ms;
          scheduler.update_interval(update_ms);
        }

      const clock::time_point end = clock::now();

      if (end - start < tick_interval)
        std::this_thread::sleep_for(tick_interval - (end - start));
    }

  std::cout << "Quit.\n";

  iscool::schedule::finalize();

  return EXIT_SUCCESS;
}
