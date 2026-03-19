// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/bot.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/constant/default_crate_probability.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/contest_timeline_writer.hpp>
#include <bim/game/dump_arena.hpp>
#include <bim/game/feature_flags_string.hpp>
#include <bim/game/player_action.hpp>

#include <bim/assume.hpp>
#include <bim/tracy.hpp>
#include <bim/version.hpp>

#include <iscool/log/enable_console_log.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/program_options.hpp>

#include <fmt/format.h>

#include <iostream>
#include <optional>
#include <random>

namespace
{
  using bot_vector =
      boost::container::static_vector<bim::game::bot,
                                      bim::game::g_max_player_count>;

  struct options
  {
    std::uint64_t seed;
    std::uint8_t player_count;
    bim::game::feature_flags features;
    std::string output_file;
    bool timeline;
    bool console_log;
  };

  struct command_line
  {
    std::optional<::options> options;
    bool valid;
  };
}

static std::string feature_list()
{
  std::string result;
  const char* separator = "";
  for (const bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    {
      result += separator;
      separator = ", ";
      result.append(bim::game::to_simple_string(f));
    }

  return result;
}

namespace bim::game
{
  std::istream& operator>>(std::istream& in, feature_flags& flags)
  {
    flags = {};
    std::string s;

    while (std::getline(in, s, ','))
      {
        const std::optional<feature_flags> f = from_simple_string(s);

        if (!f)
          throw boost::program_options::invalid_option_value(s);

        flags |= *f;
      }

    // Make sure in.good() returns true if we have read the whole input,
    // otherwise Boost.program_options would consider that the value was
    // invalid.
    if (in.eof())
      in.clear();

    return in;
  }

  std::ostream& operator<<(std::ostream& out, feature_flags flags)
  {
    const char* separator = "";

    for (const bim::game::feature_flags f :
         bim::game::g_all_game_feature_flags)
      if (!!(flags & f))
        {
          out << separator;
          separator = ",";
          out << to_simple_string(f);
        }

    return out;
  }
}

static command_line parse_command_line(int argc, char* argv[])
{
  boost::program_options::options_description options("Options");
  options.add_options()(
      "features",
      boost::program_options::value<bim::game::feature_flags>()->default_value(
          bim::game::feature_flags{}),
      fmt::format("Comma-separated list of game features to enable. Valid "
                  "values are: {}",
                  feature_list())
          .c_str());
  options.add_options()("player-count",
                        boost::program_options::value<int>()->default_value(2),
                        "The number of players in the game, 2 to 4.");
  options.add_options()("console-log", "Display logs in the terminal.");
  options.add_options()("help,h", "Display this information.");
  options.add_options()("seed", boost::program_options::value<std::uint64_t>(),
                        "The seed to use to initialize the game. Default is "
                        "to pick a random value.");
  options.add_options()(
      "output-file",
      boost::program_options::value<std::string>()->default_value("-"),
      "Where to save the game state at each iteration. Pass '-' to write on "
      "stdout.");
  options.add_options()("timeline", "Output a (binary) timeline. Default is "
                                    "to dump the game state in text.");
  options.add_options()("version", "Display the version number and exit.");

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

  if (variables.count("version") != 0)
    {
      std::cout << "Bim! Bot test " << bim::version << ".\n";
      return command_line{ .options = std::nullopt, .valid = true };
    }

  ::options result;

  if (variables.count("seed") != 0)
    result.seed = variables["seed"].as<std::uint64_t>();
  else
    result.seed = std::random_device()();

  const int player_count = variables["player-count"].as<int>();

  if ((player_count < 2) || (player_count > 4))
    {
      std::cerr << "--player-count should be 2, 3, or 4.\n";
      return command_line{ .options = std::nullopt, .valid = false };
    }

  result.player_count = player_count;
  result.features = variables["features"].as<bim::game::feature_flags>();

  if (variables.count("output-file") == 0)
    result.output_file = "-";
  else
    result.output_file = variables["output-file"].as<std::string>();

  result.timeline = variables.count("timeline") != 0;
  result.console_log = (variables.count("console-log") != 0);

  return command_line{ .options = std::move(result), .valid = true };
}

template <typename F>
static void run_contest(bot_vector& bots, bim::game::contest& contest,
                        F&& on_tick_begins)
{
  bim::game::contest_result result;
  entt::registry& registry = contest.registry();

  do
    {
      for (bim::game::bot& bot : bots)
        {
          bim::game::player_action* const a =
              bim::game::find_player_action_by_index(registry,
                                                     bot.player_index());
          if (a)
            *a = bot.think(contest);
        }

      on_tick_begins();
      result = contest.tick();
      FrameMark;
    }
  while (result.still_running());
}

int main(int argc, char* argv[])
{
  const ::command_line command_line = parse_command_line(argc, argv);

  if (!command_line.valid)
    return EXIT_FAILURE;

  if (!command_line.options)
    return EXIT_SUCCESS;

  const ::options& options = *command_line.options;

  if (options.console_log)
    iscool::log::enable_console_log();

  const bim::game::contest_fingerprint fingerprint = {
    .seed = options.seed,
    .features = options.features,
    .player_count = options.player_count,
    .crate_probability = bim::game::g_default_crate_probability,
    .arena_width = bim::game::g_default_arena_width,
    .arena_height = bim::game::g_default_arena_height
  };

  bot_vector bots;
  for (int i = 0; i != fingerprint.player_count; ++i)
    bots.emplace_back(i, fingerprint.arena_width, fingerprint.arena_height,
                      options.seed + i);

  bim::game::contest contest(fingerprint);

  std::FILE* const output_file =
      (options.output_file == "-")
          ? stdout
          : std::fopen(options.output_file.c_str(), "w");

  if (options.timeline)
    {
      bim::game::contest_timeline_writer writer(output_file, fingerprint);

      run_contest(bots, contest,
                  [&]()
                    {
                      writer.push(contest.registry());
                    });

      // contest_timeline_writer closes the output file here.
    }
  else
    {
      const std::string page_separator(80, '-');
      std::size_t t = 0;

      run_contest(bots, contest,
                  [&]()
                    {
                      fprintf(output_file, "When tick #%zu begins.\n", t);
                      ++t;

                      bim::game::dump_arena(
                          output_file, contest.arena(), contest.entity_map(),
                          contest.context(), contest.registry());
                      fprintf(output_file, "%s\n", page_separator.c_str());
                    });

      fprintf(output_file, "Final state.\n");
      bim::game::dump_arena(output_file, contest.arena(), contest.entity_map(),
                            contest.context(), contest.registry());
      fprintf(output_file, "%s\n", page_separator.c_str());

      fprintf(output_file, "%zu ticks.\n", t);

      if (output_file != stdout)
        std::fclose(output_file);
    }

  return EXIT_SUCCESS;
}
