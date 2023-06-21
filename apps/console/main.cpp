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
#include <bim/app/console/application.hpp>
#include <bim/app/console/offline_game.hpp>
#include <bim/app/console/online_game.hpp>
#include <bim/app/console/scoped_terminal_setup.hpp>

#include <iscool/schedule/manual_scheduler.h>
#include <iscool/schedule/setup.h>

#include <boost/program_options.hpp>

#include <chrono>
#include <cstdlib>
#include <memory>

namespace
{
  struct options
  {
    std::string host = "localhost:23899";
    std::string game_name = "test";
    bool offline = false;
  };
}

static std::optional<options> parse_program_options(int argc, char** argv)
{
  options result;

  boost::program_options::options_description description("Options");

  description.add_options()("help,h", "Display this message and exit.");
  description.add_options()(
      "host",
      boost::program_options::value<std::string>(&result.host)
          ->default_value(result.host)
          ->value_name("SERVER:PORT"),
      "The server hosting the online game.");
  description.add_options()(
      "game-name",
      boost::program_options::value<std::string>(&result.game_name)
          ->default_value(result.game_name)
          ->value_name("STR"),
      "The name of the online game to join.");
  description.add_options()("offline",
                            "Run an offline game instead of an online one.");

  boost::program_options::variables_map arguments;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, description),
      arguments);
  boost::program_options::notify(arguments);

  if (arguments.count("help") != 0)
    {
      std::cout << description << '\n';
      return std::nullopt;
    }

  if (arguments.count("offline") != 0)
    result.offline = true;

  return result;
}

static void run_main_loop(const bim::app::console::application& application,
                          iscool::schedule::manual_scheduler& scheduler)
{
  std::chrono::nanoseconds start
      = std::chrono::steady_clock::now().time_since_epoch();

  scheduler.update_interval(std::chrono::seconds(0));

  while (!application.should_quit())
    {
      const std::chrono::nanoseconds update_interval
          = application.update_interval();

      std::chrono::nanoseconds end
          = std::chrono::steady_clock::now().time_since_epoch();

      if (end - start < update_interval)
        std::this_thread::sleep_for(update_interval - (end - start));

      start = std::chrono::steady_clock::now().time_since_epoch();
      scheduler.update_interval(update_interval);
    }
}

static std::unique_ptr<bim::app::console::online_game>
build_online_game(bim::app::console::application& application,
                  const std::string& host, const std::string& game_name)
{
  bim::net::game_name name{};
  std::copy(game_name.begin(), game_name.end(), name.begin());

  return std::make_unique<bim::app::console::online_game>(application, host,
                                                          name);
}

int main(int argc, char** argv)
{
  const std::optional<options> options = parse_program_options(argc, argv);

  if (!options)
    return EXIT_FAILURE;

  const bim::app::console::scoped_terminal_setup no_echo(~(ICANON | ECHO));

  iscool::schedule::manual_scheduler scheduler;
  iscool::schedule::scoped_scheduler_delegate scheduler_initializer(
      scheduler.get_delayed_call_delegate());
  bim::app::console::application application;

  std::unique_ptr<bim::app::console::offline_game> offline_game;
  std::unique_ptr<bim::app::console::online_game> online_game;

  if (options->offline)
    offline_game
        = std::make_unique<bim::app::console::offline_game>(application);
  else
    online_game
        = build_online_game(application, options->host, options->game_name);

  run_main_loop(application, scheduler);

  return EXIT_SUCCESS;
}
