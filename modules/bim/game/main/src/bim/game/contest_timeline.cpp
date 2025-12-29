// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_timeline.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/kick_event.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/meta/underlying_type.hpp>
#include <iscool/net/endianness.hpp>

#include <entt/entity/registry.hpp>

#include <cassert>
#include <cstring>
#include <iostream>

bim::game::contest_timeline::contest_timeline() = default;
bim::game::contest_timeline::~contest_timeline() = default;

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

  const auto read = []<typename T>(FILE* f, T& d) -> bool
  {
    if (std::fread(&d, sizeof(char), sizeof(d), f) != sizeof(d))
      return false;

    d = (T)iscool::net::to_host_endianness(
        (typename iscool::meta::underlying_type<T>::type)d);

    return true;
  };

  std::uint32_t file_version;

  if (!read(f, file_version))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the file format version.");
      return false;
    }

  if (file_version != 2)
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Unsupported file format version {}.", file_version);
      return false;
    }

  if (!read(f, timeline.m_fingerprint.seed))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the game's seed.");
      return false;
    }

  if (!read(f, timeline.m_fingerprint.features))
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
             "than {}.",
             bim::game::g_max_player_count);
      return false;
    }

  if (!read(f, timeline.m_fingerprint.crate_probability))
    {
      ic_log(iscool::log::nature::error(), "load_contest_timeline",
             "Failed to read the crate probability.");
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

  timeline.m_kick_event_count = 0;

  const int player_count = timeline.m_fingerprint.player_count;
  int remaining_actions_in_tick = player_count;
  std::uint32_t tick = 0;

  while (std::size_t n = std::fread(buffer, 1, 4096, f))
    for (std::size_t i = 0; i != n; ++i)
      {
        {
          const std::byte byte = (std::byte)buffer[i];
          const std::byte low = byte & (std::byte)0xf;
          const std::byte high = (byte & (std::byte)0xf0) >> 4;

          if (low == (std::byte)0xf)
            {
              // Special event. We only have one kind of event, which is to
              // kick the player out.
              timeline.m_kick_event_tick[timeline.m_kick_event_count] = tick;
              timeline.m_kick_event_player[timeline.m_kick_event_count] =
                  (std::uint8_t)high;
              ++timeline.m_kick_event_count;
            }
          else
            {
              timeline.m_actions.push_back(bim::game::player_action{
                  .movement = (bim::game::player_movement)(low >> 1),
                  .drop_bomb = (bool)(low & (std::byte)1) });
              --remaining_actions_in_tick;

              if (remaining_actions_in_tick)
                {
                  timeline.m_actions.push_back(bim::game::player_action{
                      .movement = (bim::game::player_movement)(high >> 1),
                      .drop_bomb = (bool)(high & (std::byte)1) });
                  --remaining_actions_in_tick;
                }

              if (remaining_actions_in_tick == 0)
                {
                  ++tick;
                  remaining_actions_in_tick = player_count;
                }
            }
        }
      }
  return std::feof(f);
}

const bim::game::contest_fingerprint&
bim::game::contest_timeline::fingerprint() const
{
  return m_fingerprint;
}

std::size_t bim::game::contest_timeline::tick_count() const
{
  return m_actions.empty() ? 0
                           : (m_actions.size() / m_fingerprint.player_count);
}

void bim::game::contest_timeline::load_tick(std::uint32_t tick,
                                            entt::registry& registry) const
{
  assert(!m_actions.empty());
  assert(m_fingerprint.player_count != 0);
  assert(tick < m_actions.size() / m_fingerprint.player_count);

  const std::size_t kick_index =
      std::find(m_kick_event_tick.begin(),
                m_kick_event_tick.begin() + m_kick_event_count, tick)
      - m_kick_event_tick.begin();

  if (kick_index != m_kick_event_count)
    for (auto&& [entity, player, action] :
         registry.view<player, player_action>().each())
      if (player.index == m_kick_event_player[kick_index])
        kick_player(registry, m_kick_event_player[kick_index]);
      else
        action = m_actions[tick * m_fingerprint.player_count + player.index];
  else
    for (auto&& [entity, player, action] :
         registry.view<player, player_action>().each())
      action = m_actions[tick * m_fingerprint.player_count + player.index];
}
