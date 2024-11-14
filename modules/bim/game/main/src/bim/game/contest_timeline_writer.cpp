// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_timeline_writer.hpp>

#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest_fingerprint.hpp>

#include <bim/assume.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/net/endianness.hpp>

#include <utility>

bim::game::contest_timeline_writer::contest_timeline_writer()
  : m_file(nullptr)
{}

bim::game::contest_timeline_writer::contest_timeline_writer(
    std::filesystem::path file_path, const contest_fingerprint& contest)
  : m_file(std::fopen(file_path.c_str(), "w"))
{
  if (!m_file)
    return;

  constexpr const char magic[] = { 'B', 'I', 'M', '!' };
  if (std::fwrite(magic, sizeof(char), sizeof(magic), m_file) != sizeof(magic))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the magic number.");

  const auto write = [](FILE* f, auto d) -> bool
  {
    auto s = iscool::net::to_network_endianness(d);
    return std::fwrite(&s, sizeof(char), sizeof(s), f) == sizeof(s);
  };

  const std::uint32_t file_version = 1;

  if (!write(m_file, file_version))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the file format version.");

  if (!write(m_file, contest.seed))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the game's seed.");

  if (!write(m_file, contest.feature_mask))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the game's features.");

  if (!write(m_file, contest.player_count))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the player_count.");

  if (!write(m_file, contest.brick_wall_probability))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the brick wall probability.");

  if (!write(m_file, contest.arena_width))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the arena's width.");

  if (!write(m_file, contest.arena_height))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the arena's height.");
}

bim::game::contest_timeline_writer::contest_timeline_writer(
    contest_timeline_writer&& that) noexcept
  : m_file(std::exchange(that.m_file, nullptr))
{}

bim::game::contest_timeline_writer::~contest_timeline_writer()
{
  if (m_file)
    std::fclose(m_file);
}

bim::game::contest_timeline_writer&
bim::game::contest_timeline_writer::operator=(
    contest_timeline_writer&& that) noexcept
{
  if (this == &that)
    return *this;

  if (m_file)
    std::fclose(m_file);

  m_file = std::exchange(that.m_file, nullptr);

  return *this;
}

bim::game::contest_timeline_writer::operator bool() const
{
  return m_file;
}

void bim::game::contest_timeline_writer::push(std::span<player_action> actions)
{
  bim_assume(actions.size() <= g_max_player_count);
  bim_assume(actions.size() >= 2);

  std::byte serialized[g_max_player_count] = {};

  // A player_action is stored in a nibble: three bits for the movement then
  // one bit for the bomb dropping. Even player indices are in the low bits of
  // the bytes, odd player indices in the high bits: [ P1, P0 ] [ P3, P2 ]. For
  // an hypothetical one player game, P1 is zero and [ P3, P2 ] is not emitted.
  // For a three players game P3 is zero.
  for (int i = 0, n = actions.size(); i != n; ++i)
    {
      const player_action& a = actions[i];
      constexpr int bits_in_movement = 3;

      assert((int)a.movement <= ((1 << bits_in_movement) - 1));
      (void)bits_in_movement;

      std::byte b = (std::byte)a.movement;
      b = (b << 1) | (std::byte)a.drop_bomb;
      b <<= 4 * (i % 2);

      serialized[i / 2] |= b;
    }

  const std::size_t byte_count = sizeof(std::byte) * (actions.size() + 1) / 2;

  if (std::fwrite(serialized, sizeof(std::byte), byte_count, m_file)
      != byte_count)
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the actions.");
}
