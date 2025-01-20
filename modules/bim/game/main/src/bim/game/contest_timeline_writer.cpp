// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_timeline_writer.hpp>

#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest_fingerprint.hpp>

#include <bim/assume.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/meta/underlying_type.hpp>
#include <iscool/net/endianness.hpp>

#include <entt/entity/registry.hpp>

#include <utility>

static void serialize_actions(int player_count, const entt::registry& registry,
                              std::byte* buffer, std::size_t& buffer_size)
{
  std::array<bim::game::player_action, bim::game::g_max_player_count>
      actions{};

  for (auto&& [entity, player, action] :
       registry.view<bim::game::player, bim::game::player_action>().each())
    actions[player.index] = action;

  // A player_action is stored in a nibble: three bits for the movement then
  // one bit for the bomb dropping. Even player indices are in the low bits of
  // the bytes, odd player indices in the high bits: [ P1, P0 ] [ P3, P2 ]. For
  // an hypothetical one player game, P1 is zero and [ P3, P2 ] is not emitted.
  // For a three players game P3 is zero.
  for (int i = 0; i != player_count; ++i)
    {
      const bim::game::player_action& a = actions[i];
      constexpr int bits_in_movement = 3;

      assert((int)a.movement <= ((1 << bits_in_movement) - 1));
      (void)bits_in_movement;

      std::byte b = (std::byte)a.movement;
      b = (b << 1) | (std::byte)a.drop_bomb;
      b <<= 4 * (i % 2);

      buffer[buffer_size + i / 2] |= b;
    }

  buffer_size += sizeof(std::byte) * (player_count + 1) / 2;
}

static void serialize_events(const entt::registry& registry, std::byte* buffer,
                             std::size_t& buffer_size)
{
  for (auto&& [entity, player] :
       registry.view<bim::game::player, bim::game::kicked>().each())
    {
      buffer[buffer_size] = ((std::byte)player.index << 4) | (std::byte)0xf;
      ++buffer_size;
    }
}

bim::game::contest_timeline_writer::contest_timeline_writer()
  : m_file(nullptr)
{}

bim::game::contest_timeline_writer::contest_timeline_writer(
    std::FILE* file, const contest_fingerprint& contest)
  : m_file(file)
  , m_player_count(contest.player_count)
{
  if (!m_file)
    return;

  constexpr const char magic[] = { 'B', 'I', 'M', '!' };
  if (std::fwrite(magic, sizeof(char), sizeof(magic), m_file) != sizeof(magic))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the magic number.");

  const auto write = []<typename T>(FILE* f, T d) -> bool
  {
    auto s = iscool::net::to_network_endianness(
        (typename iscool::meta::underlying_type<T>::type)d);
    return std::fwrite(&s, sizeof(char), sizeof(s), f) == sizeof(s);
  };

  const std::uint32_t file_version = 2;

  if (!write(m_file, file_version))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the file format version.");

  if (!write(m_file, contest.seed))
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the game's seed.");

  if (!write(m_file, contest.features))
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
  , m_player_count(that.m_player_count)
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
  m_player_count = that.m_player_count;

  return *this;
}

bim::game::contest_timeline_writer::operator bool() const
{
  return m_file;
}

void bim::game::contest_timeline_writer::push(const entt::registry& registry)
{
  // Each action takes half a byte, thus g_max_player_count / 2 bytes max.
  // The kick events also take half a byte, so in total, for one tick, we need
  // at most g_max_player_count bytes.
  std::byte buffer[g_max_player_count] = {};
  std::size_t buffer_size = 0;

  serialize_actions(m_player_count, registry, buffer, buffer_size);
  serialize_events(registry, buffer, buffer_size);

  if (std::fwrite(buffer, sizeof(std::byte), buffer_size, m_file)
      != buffer_size)
    ic_log(iscool::log::nature::error(), "contest_timeline_writer",
           "Could not write the tick.");
}
