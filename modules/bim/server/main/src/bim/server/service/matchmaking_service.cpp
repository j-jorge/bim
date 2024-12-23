// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/matchmaking_service.hpp>

#include <bim/server/service/game_info.hpp>
#include <bim/server/service/game_service.hpp>

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
         + std::chrono::seconds(5);
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
{
  m_done_encounters.reserve(8);
}

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
    return false;

  refresh_encounter(encounter_id, encounter, endpoint, session, request_token,
                    existing_index);
  return true;
}

/**
 * Search an encounter into which the given session can be inserted. Return
 * true if the session has been included in an existing encounter.
 */
std::optional<bim::net::encounter_id>
bim::server::matchmaking_service::add_in_any_encounter(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    bim::net::client_token request_token)
{
  for (auto& [encounter_id, encounter] : m_encounters)
    {
      if (encounter.channel
          || (encounter.player_count == bim::game::g_max_player_count))
        continue;

      const std::size_t session_index = encounter.session_index(session);
      assert(session_index == encounter.sessions.size());

      refresh_encounter(encounter_id, encounter, endpoint, session,
                        request_token, session_index);
      return encounter_id;
    }

  return {};
}

void bim::server::matchmaking_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    bim::net::encounter_id encounter_id, bim::net::client_token request_token)
{
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

  // The game has not started yet, thus we can still update de player list.
  if (!encounter.channel)
    remove_inactive_sessions(encounter_id, encounter);

  int ready_count = 0;
  for (int i = 0; i != encounter.player_count; ++i)
    ready_count += encounter.ready[i];

  if ((ready_count != encounter.player_count) || (ready_count <= 1))
    return;

  // All players are ready, send the game configuration to the client.
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
             game->channel, game->fingerprint.seed);
    }

  const bim::game::contest_fingerprint& fingerprint = game->fingerprint;

  m_message_stream.send(
      endpoint,
      bim::net::launch_game(request_token, fingerprint.seed, game->channel,
                            fingerprint.feature_mask, fingerprint.player_count,
                            game->session_index(session),
                            fingerprint.brick_wall_probability,
                            fingerprint.arena_width, fingerprint.arena_height)
          .build_message(),
      session, 0);
}

std::span<const bim::net::encounter_id>
bim::server::matchmaking_service::garbage_encounters() const
{
  return m_done_encounters;
}

std::span<const bim::server::matchmaking_service::kick_session_event>
bim::server::matchmaking_service::garbage_sessions() const
{
  return m_done_sessions;
}

void bim::server::matchmaking_service::drop_garbage()
{
  m_done_sessions.clear();
  m_done_encounters.clear();
}

void bim::server::matchmaking_service::refresh_encounter(
    bim::net::encounter_id encounter_id, encounter_info& encounter,
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    bim::net::client_token request_token, std::size_t session_index)
{
  ic_log(iscool::log::nature::info(), "matchmaking_service",
         "Refreshing encounter %d with %d players on request of session %d.",
         encounter_id, (int)encounter.player_count, session);

  // Update for a player on hold.
  if (session_index < encounter.sessions.size())
    {
      encounter.release_at_this_date[session_index] = date_for_next_release();
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
        // The encounter is full, we can't do anything for the requesting
        // session.
        return;
    }

  if (encounter.player_count == 0)
    clean_up(encounter_id);
  else
    {
      encounter.clean_up_connection = schedule_clean_up(encounter_id);
      send_game_on_hold(endpoint, request_token, session, encounter_id,
                        encounter.player_count);
    }
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

        m_done_sessions.emplace_back(encounter.sessions[i], encounter_id);
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

  const encounter_map::iterator encounter_it = m_encounters.find(encounter_id);

  if (encounter_it == m_encounters.end())
    return;

  m_done_encounters.emplace_back(encounter_id);

  const encounter_info& encounter = encounter_it->second;

  for (std::size_t i = 0; i != encounter.player_count; ++i)
    m_done_sessions.emplace_back(encounter.sessions[i], encounter_id);

  m_encounters.erase(encounter_it);
}
