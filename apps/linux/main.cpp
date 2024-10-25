// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/application.hpp>

#include <bim/axmol/app/bridge.hpp>

#include <iscool/log/enable_console_log.hpp>
#include <iscool/strings/unordered_string_map.hpp>

#include <axmol/platform/Application.h>

#include <boost/container/flat_map.hpp>
#include <boost/program_options.hpp>

#include <fmt/format.h>

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <optional>

namespace
{
  struct screen_size
  {
    int width;
    int height;
  };

  struct options
  {
    screen_size screen_resolution;
    float screen_scale;
    bool console_log;
    std::vector<std::string> asset_directories;
  };

  struct command_line
  {
    std::optional<::options> options;
    bool valid;
  };

  using device_screen_map =
      boost::container::flat_map<std::string, screen_size, std::less<>>;
}

static device_screen_map g_device_screens = { { "iphone15", { 1179, 2556 } },
                                              { "pixel3a", { 1080, 2220 } },
                                              { "nexus4", { 768, 1280 } } };

static bool parse_screen_size(screen_size& size, std::string_view id)
{
  screen_size parsed_size;

  const char* cursor = id.data();
  const char* const end = cursor + id.size();
  std::from_chars_result parse_result =
      std::from_chars(cursor, end, parsed_size.width);

  if (parse_result.ec == std::errc{})
    {
      cursor = parse_result.ptr;

      if ((cursor != end) && (*cursor == 'x'))
        {
          parse_result = std::from_chars(cursor + 1, end, parsed_size.height);
          if ((parse_result.ec == std::errc{}) && (parse_result.ptr == end))
            {
              size = parsed_size;
              return true;
            }
        }
    }

  const device_screen_map::const_iterator it = g_device_screens.find(id);

  if (it != g_device_screens.end())
    {
      size = it->second;
      return true;
    }

  std::cerr << fmt::format("Could not understand the screen size '{}'.\n", id);

  return false;
}

static void display_known_devices()
{
  std::cout << "Known devices and their screen resolution in pixels:\n";

  for (auto& [id, size] : g_device_screens)
    std::cout << fmt::format("  - {} ({}x{})\n", id, size.width, size.height);
}

static command_line parse_command_line(int argc, char* argv[])
{
  boost::program_options::options_description options("Options");
  options.add_options()("help,h", "Display this information.");
  options.add_options()(
      "screen",
      boost::program_options::value<std::string>()->default_value("pixel3a"),
      "The screen resolution, in pixels, of the targetted device. It can be "
      "either WIDTHxHEIGHT (e.g. 720x1280) or the name of a device. Pass "
      "--screen list to get a list of known devices.");
  options.add_options()(
      "scale", boost::program_options::value<float>()->default_value(1),
      "The scale to apply to the game window.");
  options.add_options()(
      "assets",
      boost::program_options::value<std::vector<std::string>>()
          ->value_name("path...")
          ->multitoken(),
      "Directories where the game assets can be found. Assets are searched "
      "in these directories, in the provided order.");
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
      return command_line{ .options = std::nullopt, .valid = true };
    }

  boost::program_options::variables_map::const_iterator asset_directories =
      variables.find("assets");

  if (asset_directories == variables.end())
    {
      std::cerr << "Missing an --assets option. See --help for details.\n";
      return command_line{};
    }

  ::options result;

  result.asset_directories =
      asset_directories->second.as<std::vector<std::string>>();
  result.console_log = (variables.count("console-log") != 0);

  if (variables.count("screen") != 0)
    {
      const std::string& screen_id = variables["screen"].as<std::string>();

      if (screen_id == "list")
        {
          display_known_devices();
          return command_line{ std::nullopt, true };
        }

      if (!parse_screen_size(result.screen_resolution, screen_id))
        return command_line{};
    }

  result.screen_scale = variables["scale"].as<float>();

  if (result.screen_scale <= 0)
    {
      std::cerr << fmt::format("--scale must be a positive float, not {}.\n",
                               result.screen_scale);
      return command_line{};
    }

  return command_line{ .options = std::move(result), .valid = true };
}

int main(int argc, char* argv[])
{
  std::unique_ptr<bim::axmol::app::bridge> bridge(
      new bim::axmol::app::bridge());

  const ::command_line command_line = parse_command_line(argc, argv);

  if (!command_line.valid)
    return EXIT_FAILURE;

  if (!command_line.options)
    return EXIT_SUCCESS;

  const ::options& options = *command_line.options;

  if (options.console_log)
    iscool::log::enable_console_log();

  bim::axmol::app::application app(options.asset_directories,
                                   ax::Size(options.screen_resolution.width,
                                            options.screen_resolution.height),
                                   options.screen_scale);

  const int result = axmol::Application::getInstance()->run();

  // The bridge must be destroyed before the app becauses it accesses the
  // global scheduler, which is destroyed with the app.
  bridge.reset();

  return result;
}
