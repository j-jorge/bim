/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bim/server/service/matchmaking_service.hpp>

#include <bim/server/service/game_info.hpp>
#include <bim/server/service/game_service.hpp>

#include <bim/net/message/accept_game.hpp>
#include <bim/net/message/game_on_hold.hpp>
#include <bim/net/message/launch_game.hpp>

#include <bim/game/constant/max_player_count.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

#include <algorithm>
#include <cassert>
#include <optional>

static std::chrono::nanoseconds date_for_next_release()
{
  return iscool::time::now<std::chrono::nanoseconds>()
         + std::chrono::minutes(1);
}

struct bim::server::matchmaking_service::encounter_info
{
  std::uint8_t player_count;
  std::array<iscool::net::session_id, bim::game::g_max_player_count> sessions;
  std::array<std::chrono::nanoseconds, bim::game::g_max_player_count>
      release_at_this_date;
  std::array<bool, bim::game::g_max_player_count> ready;
  iscool::signals::scoped_connection clean_up_connection;
  std::optional<iscool::net::channel_id> channel;

  void erase(std::size_t i)
  {
    assert(i < player_count);

    std::copy(sessions.begin() + i + 1, sessions.begin() + player_count,
              sessions.begin() + i);
    std::copy(release_at_this_date.begin() + i + 1,
              release_at_this_date.begin() + player_count,
              release_at_this_date.begin() + i);
    std::copy(ready.begin() + i + 1, ready.begin() + player_count,
              ready.begin() + i);
    --player_count;
  }

  size_t session_index(iscool::net::session_id session) const
  {
    return std::find(sessions.begin(), sessions.end(), session)
           - sessions.begin();
  }
};

bim::server::matchmaking_service::matchmaking_service(
    iscool::net::socket_stream& socket, game_service& game_service)
  : m_message_stream(socket)
  , m_game_service(game_service)
  , m_next_encounter_id(1)
{}

bim::server::matchmaking_service::~matchmaking_service() = default;

bim::net::encounter_id bim::server::matchmaking_service::new_encounter(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    bim::net::client_token request_token)
{
  ic_log(iscool::log::nature::info(), "matchmaking_service",
         "Creating new encounter %d on request of session %d.",
         m_next_encounter_id, session);

  const bim::net::encounter_id encounter_id = m_next_encounter_id;
  ++m_next_encounter_id;

  assert(m_encounters.find(encounter_id) == m_encounters.end());
  encounter_info& encounter = m_encounters[encounter_id];

  encounter.player_count = 1;
  encounter.sessions[0] = session;
  encounter.release_at_this_date[0] = date_for_next_release();
  encounter.clean_up_connection = schedule_clean_up(encounter_id);
  encounter.ready.fill(false);

  send_game_on_hold(endpoint, request_token, session, encounter_id,
                    encounter.player_count);

  return encounter_id;
}

/// Return true if the session has been included in the given encounter.
bool bim::server::matchmaking_service::refresh_encounter(
    bim::net::encounter_id encounter_id, const iscool::net::endpoint& endpoint,
    iscool::net::session_id session, bim::net::client_token request_token)
{
  const encounter_map::iterator it = m_encounters.find(encounter_id);

  if (it == m_encounters.end())
    return false;

  encounter_info& encounter = it->second;

  const std::size_t existing_index = encounter.session_index(session);

  // No update of the participants once the game has started.
  if (encounter.channel)
    return (existing_index != encounter.sessions.size())
           && m_game_service.is_playing(*encounter.channel);

  ic_log(iscool::log::nature::info(), "matchmaking_service",
         "Refreshing encounter '%d' with %d players on request of session %d.",
         encounter_id, (int)encounter.player_count, session);

  // Update for a player on hold.
  if (existing_index < encounter.sessions.size())
    {
      encounter.release_at_this_date[existing_index] = date_for_next_release();
      remove_inactive_sessions(encounter_id, encounter);
    }
  else
    {
      remove_inactive_sessions(encounter_id, encounter);

      if (encounter.player_count != encounter.sessions.size())
        {
          const std::size_t new_index = encounter.player_count;
          encounter.sessions[new_index] = session;
          encounter.release_at_this_date[new_index] = date_for_next_release();
          ++encounter.player_count;
        }
      else
        return false;
    }

  if (encounter.player_count == 0)
    clean_up(encounter_id);
  else
    {
      encounter.clean_up_connection = schedule_clean_up(encounter_id);
      send_game_on_hold(endpoint, request_token, session, encounter_id,
                        encounter.player_count);
    }

  return true;
}

void bim::server::matchmaking_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::accept_game& message)
{
  const bim::net::encounter_id encounter_id = message.get_encounter_id();

  ic_log(iscool::log::nature::info(), "matchmaking_service",
         "Accepted game. Session %d, encounter %d.", session, encounter_id);

  const encounter_map::iterator it = m_encounters.find(encounter_id);

  if (it == m_encounters.end())
    {
      ic_log(iscool::log::nature::info(), "matchmaking_service",
             "Game %d does not exist.", encounter_id);
      return;
    }

  encounter_info& encounter = it->second;
  const std::size_t existing_index = encounter.session_index(session);

  // Update for a player on hold.
  if (existing_index == encounter.sessions.size())
    {
      ic_log(iscool::log::nature::info(), "matchmaking_service",
             "Session %d is not part of encounter %d.", session, encounter_id);
      return;
    }

  encounter.release_at_this_date[existing_index] = date_for_next_release();
  encounter.ready[existing_index] = true;
  encounter.clean_up_connection = schedule_clean_up(encounter_id);

  if (!encounter.channel)
    remove_inactive_sessions(encounter_id, encounter);

  int ready_count = 0;
  for (int i = 0; i != encounter.player_count; ++i)
    ready_count += encounter.ready[i];

  if ((ready_count != encounter.player_count) || (ready_count <= 1))
    return;

  std::optional<game_info> game;

  if (encounter.channel)
    {
      game = m_game_service.find_game(*encounter.channel);

      if (!game)
        return;
    }
  else
    {
      game =
          m_game_service.new_game(encounter.player_count, encounter.sessions);
      encounter.channel = game->channel;

      ic_log(iscool::log::nature::info(), "matchmaking_service",
             "Channel for encounter %d is %d, seed %d.", it->first,
             game->channel, game->seed);
    }

  constexpr std::uint32_t feature_mask = 0;
  constexpr std::uint8_t brick_wall_probability = 80;
  constexpr std::uint8_t arena_width = 13;
  constexpr std::uint8_t arena_height = 15;

  m_message_stream.send(
      endpoint,
      bim::net::launch_game(message.get_request_token(), game->seed,
                            game->channel, feature_mask, game->player_count,
                            game->session_index(session),
                            brick_wall_probability, arena_width, arena_height)
          .build_message(),
      session, 0);
}

void bim::server::matchmaking_service::send_game_on_hold(
    const iscool::net::endpoint& endpoint, bim::net::client_token token,
    iscool::net::session_id session, bim::net::encounter_id encounter_id,
    std::uint8_t player_count)
{
  m_message_stream.send(
      endpoint,
      bim::net::game_on_hold(token, encounter_id, player_count)
          .build_message(),
      session, 0);
}

void bim::server::matchmaking_service::remove_inactive_sessions(
    bim::net::encounter_id encounter_id, encounter_info& encounter)
{
  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();

  for (int i = 0; i != encounter.player_count;)
    if (encounter.release_at_this_date[i] <= now)
      {
        ic_log(iscool::log::nature::info(), "matchmaking_service",
               "Kicking %d from %d: timeout.", encounter.sessions[i],
               encounter_id);
        encounter.erase(i);
      }
    else
      ++i;
}

iscool::signals::connection
bim::server::matchmaking_service::schedule_clean_up(
    bim::net::encounter_id encounter_id)
{
  return iscool::schedule::delayed_call(
      [this, encounter_id]() -> void
      {
        clean_up(encounter_id);
      },
      std::chrono::minutes(3));
}

void bim::server::matchmaking_service::clean_up(
    bim::net::encounter_id encounter_id)
{
  ic_log(iscool::log::nature::info(), "matchmaking_service",
         "Cleaning up encounter %d.", encounter_id);

  m_encounters.erase(encounter_id);
}
