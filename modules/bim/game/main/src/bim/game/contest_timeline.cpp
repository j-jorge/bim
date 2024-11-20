// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_timeline.hpp>

#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/max_player_count.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/net/endianness.hpp>

#include <cassert>
#include <cstring>
#include <iostream>

bool bim::game::load_contest_timeline(contest_timeline& timeline, std::FILE* f)
{
  char buffer[4096];
  constexpr const char expected_magic[] = { 'B', 'I', 'M', '!' };
  constexpr std::size_t magic_length = sizeof(expected_magic);

  if (std::fread(buffer, sizeof(char), magic_length, f) != magic_length)
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Could not read magic number from file.");
      return false;
    }

  if (std::strncmp(buffer, expected_magic, magic_length) != 0)
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Magic number does not match.");
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
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the file format version.");
      return false;
    }

  if (file_version != 1)
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Unsupported file format version %d.", file_version);
      return false;
    }

  if (!read(f, timeline.m_fingerprint.seed))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the game's seed.");
      return false;
    }

  if (!read(f, timeline.m_fingerprint.feature_mask))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the game's features.");
      return false;
    }

  if (!read(f, timeline.m_fingerprint.player_count))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the player count.");
      return false;
    }

  if (timeline.m_fingerprint.player_count == 0)
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "there is no player in this game.");
      return false;
    }

  if (timeline.m_fingerprint.player_count >= bim::game::g_max_player_count)
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "there are too many players in this game, I cannot handle more "
             "than %d.",
             bim::game::g_max_player_count);
      return false;
    }

  if (!read(f, timeline.m_fingerprint.brick_wall_probability))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the brick wall probability.");
      return false;
    }

  if (!read(f, timeline.m_fingerprint.arena_width))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the arena's width.");
      return false;
    }

  if (!read(f, timeline.m_fingerprint.arena_height))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the arena's height.");
      return false;
    }

  const std::size_t byte_count_per_tick =
      (timeline.m_fingerprint.player_count + 1) / 2;
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
              bits_left = 8;
            }

          const std::byte nibble = byte & (std::byte)0xf;
          byte >>= 4;
          bits_left -= 4;

          timeline.m_actions.push_back(bim::game::player_action{
              .movement = (bim::game::player_movement)(nibble >> 1),
              .drop_bomb = (bool)(nibble & (std::byte)1) });

          if (bits_left == 0)
            ++i;
        }
    }

  return std::feof(f);
}

const bim::game::contest_fingerprint&
bim::game::contest_timeline::fingerprint() const
{
  return m_fingerprint;
}

std::span<const bim::game::player_action>
bim::game::contest_timeline::tick(std::size_t i) const
{
  assert(!m_actions.empty());
  assert(m_fingerprint.player_count != 0);
  assert(i < m_actions.size() / m_fingerprint.player_count);

  return std::span(m_actions.begin() + i * m_fingerprint.player_count,
                   m_fingerprint.player_count);
}

std::size_t bim::game::contest_timeline::tick_count() const
{
  return m_actions.empty() ? 0
                           : (m_actions.size() / m_fingerprint.player_count);
}
