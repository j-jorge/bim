// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/config.hpp>
#include <bim/server/server.hpp>

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

static command_line parse_command_line(int argc, char* argv[])
{
  boost::program_options::options_description options("Options");
  options.add_options()("console-log", "Display logs in the terminal.");
  options.add_options()("help,h", "Display this information.");
  options.add_options()("log-file",
                        boost::program_options::value<std::string>(),
                        "The file into which to append the logs.");
  options.add_options()(
      "port",
      boost::program_options::value<unsigned short>()->default_value(23899),
      "The port to listen on.");
  options.add_options()(
      "contest-timeline-folder", boost::program_options::value<std::string>(),
      "Enable the recording of the contests and save them in this folder.");

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
  result.config.port = variables["port"].as<unsigned short>();

  result.config.enable_contest_timeline_recording =
      variables.count("contest-timeline-folder") != 0;

  if (result.config.enable_contest_timeline_recording)
    result.config.contest_timeline_folder =
        variables["contest-timeline-folder"].as<std::string>();

  if (variables.count("log-file") != 0)
    result.log_to_file = variables["log-file"].as<std::string>();

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
