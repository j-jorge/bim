// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/dump_arena.hpp>

#include <iscool/net/endianness.hpp>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

namespace
{
  struct contest_timeline
  {
    contest_timeline() = default;
    contest_timeline(const contest_timeline&) = delete;

    bim::game::contest_fingerprint fingerprint;
    std::vector<std::span<const bim::game::player_action>> ticks;
    std::vector<bim::game::player_action> actions;
  };
}

static bool load_timeline(contest_timeline& timeline, FILE* f)
{
  char buffer[4096];
  constexpr const char expected_magic[] = { 'B', 'I', 'M', '!' };
  constexpr std::size_t magic_length = sizeof(expected_magic);

  if (std::fread(buffer, sizeof(char), magic_length, f) != magic_length)
    {
      std::cerr << "Error: Could not read magic number from file.\n";
      return false;
    }

  if (std::strncmp(buffer, expected_magic, magic_length) != 0)
    {
      std::cerr << "Error: Magic number does not match.\n";
      return false;
    }

  const auto read = [](FILE* f, auto& d) -> bool
  {
    if (std::fread(&d, sizeof(char), sizeof(d), f) != sizeof(d))
      return false;

    d = iscool::net::to_host_endianness(d);

    return true;
  };

  std::uint32_t file_version;

  if (!read(f, file_version))
    {
      std::cerr << "Error: Failed to read the file format version.\n";
      return false;
    }

  if (file_version != 1)
    {
      std::cerr << "Error: Unsupported file format version " << file_version
                << ".\n";
      return false;
    }

  if (!read(f, timeline.fingerprint.seed))
    {
      std::cerr << "Error: Failed to read the game's seed.\n";
      return false;
    }

  if (!read(f, timeline.fingerprint.feature_mask))
    {
      std::cerr << "Error: Failed to read the game's features.\n";
      return false;
    }

  if (!read(f, timeline.fingerprint.player_count))
    {
      std::cerr << "Error: Failed to read the player count.\n";
      return false;
    }

  if (timeline.fingerprint.player_count == 0)
    {
      std::cerr << "Error: there is no player in this game.\n";
      return false;
    }

  if (timeline.fingerprint.player_count >= bim::game::g_max_player_count)
    {
      std::cerr << "Error: there are too many players in this game, I cannot "
                   "handle more than"
                << (int)bim::game::g_max_player_count << ".\n";
      return false;
    }

  if (!read(f, timeline.fingerprint.brick_wall_probability))
    {
      std::cerr << "Error: Failed to read the brick wall probability.\n";
      return false;
    }

  if (!read(f, timeline.fingerprint.arena_width))
    {
      std::cerr << "Error: Failed to read the arena's width.\n";
      return false;
    }

  if (!read(f, timeline.fingerprint.arena_height))
    {
      std::cerr << "Error: Failed to read the arena's height.\n";
      return false;
    }

  const std::size_t byte_count_per_tick =
      (timeline.fingerprint.player_count + 1) / 2;
  assert(byte_count_per_tick > 0);

  const std::size_t ticks_per_buffer = sizeof(buffer) / byte_count_per_tick;
  assert(ticks_per_buffer > 0);

  while (std::size_t n =
             std::fread(buffer, byte_count_per_tick, ticks_per_buffer, f))
    {
      int bits_left = 0;
      std::byte byte;

      for (std::size_t i = 0; i != n * byte_count_per_tick;)
        {
          if (bits_left == 0)
            {
              byte = (std::byte)buffer[i];
              ++i;
              bits_left = 8;
            }

          const std::byte nibble = byte & (std::byte)0xf;
          byte >>= 4;
          bits_left -= 4;

          timeline.actions.push_back(bim::game::player_action{
              .movement = (bim::game::player_movement)(nibble >> 1),
              .drop_bomb = (bool)(nibble & (std::byte)1) });
        }
    }

  timeline.ticks.reserve(timeline.actions.size()
                         / timeline.fingerprint.player_count);

  for (std::size_t i = 0, n = timeline.actions.size(); i < n;
       i += timeline.fingerprint.player_count)
    timeline.ticks.push_back(std::span(timeline.actions.begin() + i,
                                       timeline.fingerprint.player_count));

  return std::feof(f);
};

static void dump_timeline(const contest_timeline& timeline)
{
  const std::string page_separator(80, '-');
  std::cout << "Tick count: " << timeline.ticks.size() << ".\n"
            << "Game seed: " << timeline.fingerprint.seed << ".\n"
            << "Player count: " << (int)timeline.fingerprint.player_count
            << ".\n"
            << page_separator << '\n';

  bim::game::contest contest(
      timeline.fingerprint.seed, timeline.fingerprint.brick_wall_probability,
      timeline.fingerprint.player_count, timeline.fingerprint.arena_width,
      timeline.fingerprint.arena_height);

  std::array<bim::game::player_action*, bim::game::g_max_player_count>
      player_actions;

  contest.registry().view<bim::game::player, bim::game::player_action>().each(
      [&player_actions](const bim::game::player& player,
                        bim::game::player_action& action)
      {
        player_actions[player.index] = &action;
      });

  std::cout << "Initial state\n";
  bim::game::dump_arena(contest.arena(), contest.registry());
  std::cout << page_separator << '\n';

  bim::game::contest_result contest_result;

  for (std::size_t t = 0, n = timeline.ticks.size(); t != n; ++t)
    {
      const std::span<const bim::game::player_action>& actions =
          timeline.ticks[t];

      for (int i = 0; i != timeline.fingerprint.player_count; ++i)
        *player_actions[i] = actions[i];

      std::cout << "When tick #" << t << " begins.\n";

      bim::game::dump_arena(contest.arena(), contest.registry());
      std::cout << page_separator << '\n';

      contest_result = contest.tick();
    }

  std::cout << "Final state.\n";
  bim::game::dump_arena(contest.arena(), contest.registry());
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

  contest_timeline timeline;

  if (!load_timeline(timeline, f))
    return EXIT_FAILURE;

  std::fclose(f);

  dump_timeline(timeline);

  return EXIT_SUCCESS;
}
