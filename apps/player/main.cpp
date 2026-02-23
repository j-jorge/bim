// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/player.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/contest_timeline.hpp>
#include <bim/game/dump_arena.hpp>
#include <bim/game/feature_flags.hpp>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

static void dump_timeline(const bim::game::contest_timeline& timeline)
{
  const bim::game::contest_fingerprint& fingerprint = timeline.fingerprint();

  const std::string page_separator(80, '-');
  std::cout << "Tick count: " << timeline.tick_count() << '\n'
            << "Game seed: " << fingerprint.seed << '\n'
            << "Player count: " << (int)fingerprint.player_count << '\n'
            << "Features: (0x" << std::hex << (int)fingerprint.features
            << ") [" << std::dec;

  const char* separator = "";
  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    {
      if (!(fingerprint.features & f))
        continue;

      std::cout << separator;
      separator = ", ";

      switch (f)
        {
        case bim::game::feature_flags::falling_blocks:
          std::cout << "falling_blocks";
          break;
        case bim::game::feature_flags::fog_of_war:
          std::cout << "fog_of_war";
          break;
        case bim::game::feature_flags::invisibility:
          std::cout << "invisibility";
          break;
        case bim::game::feature_flags::shield:
          std::cout << "shield";
          break;
        case bim::game::feature_flags::fences:
          std::cout << "fences";
          break;
        }
    }

  std::cout << "]\n" << page_separator << '\n';

  bim::game::contest contest(fingerprint);

  std::cout << "Initial state\n";
  bim::game::dump_arena(contest.arena(), contest.entity_map(),
                        contest.context(), contest.registry());
  std::cout << page_separator << '\n';

  bim::game::contest_result contest_result;

  for (std::size_t t = 0, n = timeline.tick_count(); t != n; ++t)
    {
      timeline.load_tick(t, contest.registry());

      std::cout << "When tick #" << t << " begins.\n";

      bim::game::dump_arena(contest.arena(), contest.entity_map(),
                            contest.context(), contest.registry());
      std::cout << page_separator << '\n';

      contest_result = contest.tick();
    }

  std::cout << "Final state.\n";
  bim::game::dump_arena(contest.arena(), contest.entity_map(),
                        contest.context(), contest.registry());
  std::cout << page_separator << '\n';

  if (contest_result.still_running())
    std::cout << "The game is not over.\n";
  else if (contest_result.has_a_winner())
    std::cout << "Player " << (int)contest_result.winning_player()
              << " won.\n";
  else
    std::cout << "Everybody lost.\n";
}

static void usage(std::string_view program_name)
{}

int main(int argc, char* argv[])
{
  for (int i = 1; i != argc; ++i)
    {
      const std::string_view arg = argv[i];

      if ((arg == "-h") || (arg == "--help"))
        {
          usage(argv[0]);
          return EXIT_SUCCESS;
        }
    }

  if (argc != 2)
    {
      std::cerr << "Missing file name. See --help for details.\n";
      return EXIT_FAILURE;
    }

  errno = 0;
  std::FILE* f = std::fopen(argv[1], "r");

  if (!f)
    {
      std::cerr << "Failed to open '" << argv[1] << "':" << strerror(errno)
                << '\n';
      return EXIT_FAILURE;
    }

  bim::game::contest_timeline timeline;

  if (!bim::game::load_contest_timeline(timeline, f))
    return EXIT_FAILURE;

  std::fclose(f);

  dump_timeline(timeline);

  return EXIT_SUCCESS;
}
