#include <bim/axmol/app/application.hpp>

#include <bim/axmol/app/bridge.hpp>

#include <iscool/log/enable_console_log.hpp>

#include <axmol/platform/Application.h>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <optional>

namespace
{
  struct options
  {
    bool console_log;
    std::vector<std::string> asset_directories;
  };
}

static std::optional<options> parse_command_line(int argc, char* argv[])
{
  boost::program_options::options_description options("Options");
  options.add_options()("help,h", "Display this information.");
  options.add_options()(
      "assets",
      boost::program_options::value<std::vector<std::string>>()
          ->value_name("path...")
          ->multitoken(),
      "Directories where the game assets can be found. Assets are searched in "
      "these directories, in the provided order.");
  options.add_options()("console-log", "Display logs in the terminal.");

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
      return std::nullopt;
    }

  boost::program_options::variables_map::const_iterator asset_directories =
      variables.find("assets");

  if (asset_directories == variables.end())
    {
      std::cerr << "Missing an --assets option. See --help for details.\n";
      return std::nullopt;
    }

  ::options result;

  result.asset_directories =
      asset_directories->second.as<std::vector<std::string>>();
  result.console_log = (variables.count("console-log") != 0);

  return result;
}

int main(int argc, char* argv[])
{
  bim::axmol::app::bridge bridge;

  const std::optional<options> options = parse_command_line(argc, argv);

  if (!options)
    return EXIT_FAILURE;

  if (options->console_log)
    iscool::log::enable_console_log();

  bim::axmol::app::application app(options->asset_directories);
  return axmol::Application::getInstance()->run();
}
