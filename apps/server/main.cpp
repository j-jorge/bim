// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/config.hpp>
#include <bim/server/server.hpp>

#include <iscool/json/cast_bool.hpp>
#include <iscool/json/cast_uint16.hpp>
#include <iscool/json/cast_uint64.hpp>
#include <iscool/json/is_member.hpp>
#include <iscool/json/parse_stream.hpp>
#include <iscool/log/add_file_sink.hpp>
#include <iscool/log/enable_console_log.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/setup.hpp>
#include <iscool/schedule/manual_scheduler.hpp>
#include <iscool/schedule/setup.hpp>

#include <boost/program_options.hpp>

#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>

static bool g_keep_running = true;

namespace
{
  struct options
  {
    std::string log_to_file;
    bool console_log;

    bim::server::config config;
  };

  struct command_line
  {
    std::optional<::options> options;
    bool valid;
  };
}

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

namespace
{
  template <typename T>
  struct fundamental_type
  {
    using type = T;
  };

  template <typename Rep, typename Period>
  struct fundamental_type<std::chrono::duration<Rep, Period>>
  {
    using type = Rep;
  };

  template <typename T>
  using fundamental_t = fundamental_type<T>::type;
}

template <typename T>
static void
read_config_option(std::string name, T& value,
                   boost::program_options::variables_map& variables,
                   const Json::Value& json_config)
{
  std::replace(name.begin(), name.end(), '_', '-');

  const boost::program_options::variables_map::const_iterator it =
      variables.find(name);

  if (it == variables.end())
    {
      if (iscool::json::is_member(name, json_config))
        value = T(iscool::json::cast<fundamental_t<T>>(json_config[name]));

      return;
    }

  if (it->second.defaulted() && iscool::json::is_member(name, json_config))
    {
      value = T(iscool::json::cast<fundamental_t<T>>(json_config[name]));
      return;
    }

  if constexpr (std::is_same_v<fundamental_t<T>, bool>)
    value = true;
  else
    value = T(it->second.as<fundamental_t<T>>());
}

static command_line parse_command_line(int argc, char* argv[])
{
  boost::program_options::options_description options("Options");
  options.add_options()("console-log", "Display logs in the terminal.");
  options.add_options()("help,h", "Display this information.");
  options.add_options()("log-file",
                        boost::program_options::value<std::string>(),
                        "The file into which to append the logs.");
  options.add_options()("config", boost::program_options::value<std::string>(),
                        "Load the server config from this file.");
  options.add_options()(
      "port",
      boost::program_options::value<unsigned short>()->default_value(23899),
      "The port to listen on.");
  options.add_options()(
      "authentication-clean-up-interval",
      boost::program_options::value<std::uint64_t>(),
      "Time interval in seconds at which we remove the sessions from "
      "the authentication.");
  options.add_options()("matchmaking-clean-up-interval",
                        boost::program_options::value<std::uint64_t>(),
                        "Time interval in seconds at which we remove the "
                        "encounters from the matchmaking.");
  options.add_options()(
      "random-game-auto-start-delay",
      boost::program_options::value<std::uint64_t>(),
      "How long to wait for the players to be ready before automatically"
      " launching a random game, in seconds.");
  options.add_options()(
      "game_service_clean_up_interval",
      boost::program_options::value<std::uint64_t>(),
      "Time interval at which we remove the games for which no activity has"
      " been observed, in seconds.");
  options.add_options()(
      "game-service-disconnection-lateness-threshold-in-ticks",
      boost::program_options::value<int>(),
      "How many ticks behind the second slowest client can the slowest client"
      " be before being disconnected.");
  options.add_options()(
      "game-service-disconnection-earliness-threshold-in-ticks",
      boost::program_options::value<int>(),
      "How many ticks ahead of the second fastest client can the fastest "
      "client be before being disconnected.");
  options.add_options()(
      "game-service-disconnection-inactivity-delay",
      boost::program_options::value<std::uint64_t>(),
      "How many seconds of inactivity (i.e. no message from the client) do we "
      "tolerate before disconnecting a client.");

  options.add_options()("enable-contest-timeline-recording",
                        "Tells if we record the games played in this server. "
                        "The games are saved in --contest-timeline-folder.");
  options.add_options()(
      "contest-timeline-folder", boost::program_options::value<std::string>(),
      "Path to the folder where to store the contest timelines.");

  options.add_options()("enable-geolocation",
                        "Whether or not we use IP geolocation.");
  options.add_options()("geolocation-clean-up-interval",
                        boost::program_options::value<std::uint64_t>(),
                        "How many seconds after the last request for a given "
                        "IP to be removed from the geolocation service. The "
                        "IP will receive a new ID on the next request.");
  options.add_options()("geolocation-update-interval",
                        boost::program_options::value<std::uint64_t>(),
                        "Interval in seconds at which we reopen the GeoIP "
                        "database, to get fresh data.");
  options.add_options()("geolocation-database-path",
                        boost::program_options::value<std::string>(),
                        "The path to the GeoIP database.");

  boost::program_options::variables_map variables;
  boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv)
          .options(options)
          .run(),
      variables);

  boost::program_options::notify(variables);

  if (variables.count("help") != 0)
    {
      std::cout << "Usage: " << argv[0] << " OPTIONS\n\n" << options;
      return command_line{ .options = std::nullopt, .valid = true };
    }

  ::options result;
  result.console_log = (variables.count("console-log") != 0);
  result.config.random_seed = std::random_device()();

  if (variables.count("log-file") != 0)
    result.log_to_file = variables["log-file"].as<std::string>();

  Json::Value json_config;
  const boost::program_options::variables_map::const_iterator config_path =
      variables.find("config");

  if (config_path != variables.end())
    {
      const std::string config_path_str =
          config_path->second.as<std::string>();
      std::ifstream f(config_path_str);

      if (!f)
        {
          std::cerr << "Could not load '" << config_path_str << "'.\n";
          return command_line{ .options = std::nullopt, .valid = false };
        }
      json_config = iscool::json::parse_stream(f);

      if (json_config.isNull())
        {
          std::cerr << "JSON from '" << config_path_str << "' is null.\n";
          return command_line{ .options = std::nullopt, .valid = false };
        }
    }

#define parse_config_option(c)                                                \
  do                                                                          \
    {                                                                         \
      read_config_option(#c, result.config.c, variables, json_config);        \
    }                                                                         \
  while (false)

  parse_config_option(port);
  parse_config_option(authentication_clean_up_interval);
  parse_config_option(matchmaking_clean_up_interval);
  parse_config_option(random_game_auto_start_delay);
  parse_config_option(game_service_clean_up_interval);
  parse_config_option(game_service_disconnection_lateness_threshold_in_ticks);
  parse_config_option(game_service_disconnection_earliness_threshold_in_ticks);
  parse_config_option(game_service_disconnection_inactivity_delay);

  parse_config_option(enable_contest_timeline_recording);

  if (result.config.enable_contest_timeline_recording)
    parse_config_option(contest_timeline_folder);

  parse_config_option(enable_geolocation);

  if (result.config.enable_geolocation)
    {
      parse_config_option(geolocation_clean_up_interval);
      parse_config_option(geolocation_update_interval);
      parse_config_option(geolocation_database_path);
    }

#undef parse_config_option

  return command_line{ .options = std::move(result), .valid = true };
}

int main(int argc, char* argv[])
{
  const command_line command_line = parse_command_line(argc, argv);

  if (!command_line.valid)
    return EXIT_FAILURE;

  if (!command_line.options)
    return EXIT_SUCCESS;

  install_signal_handlers();

  iscool::log::scoped_initializer log;

  if (command_line.options->console_log)
    iscool::log::enable_console_log();

  if (!command_line.options->log_to_file.empty())
    iscool::log::add_file_sink(command_line.options->log_to_file,
                               std::ios_base::app);

  iscool::schedule::manual_scheduler scheduler;
  iscool::schedule::initialize(scheduler.get_delayed_call_delegate());

  std::cout << "Press Ctrl+C to exit.\n";
  ic_log(iscool::log::nature::info(), "server", "Running on port {}.",
         command_line.options->config.port);

  bim::server::server server(command_line.options->config);

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

  ic_log(iscool::log::nature::info(), "server", "Quit.");

  iscool::schedule::finalize();

  return EXIT_SUCCESS;
}
