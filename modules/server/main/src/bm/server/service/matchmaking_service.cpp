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
#include <bm/server/server.hpp>

#include <bm/net/message/accept_game.hpp>
#include <bm/net/message/game_on_hold.hpp>
#include <bm/net/message/launch_game.hpp>
#include <bm/net/message/new_game_request.hpp>

#include <iscool/log/causeless_log.h>
#include <iscool/log/nature/info.h>
#include <iscool/time/now.h>

#include <cassert>

static std::chrono::nanoseconds date_for_next_release()
{
  return iscool::time::now<std::chrono::nanoseconds>()
         + std::chrono::minutes(1);
}

struct bm::server::matchmaking_service::game_info
{
  std::uint8_t player_count;
  std::array<iscool::net::session_id, 4> sessions;
  std::array<std::chrono::nanoseconds, 4> release_at_this_date;
  std::array<bool, 4> ready;
  iscool::signals::scoped_connection clean_up_connection;

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

bm::server::matchmaking_service::matchmaking_service(
    iscool::net::socket_stream& socket)
  : m_message_stream(socket)
  , m_next_game_id(1)
{}

bm::server::matchmaking_service::~matchmaking_service() = default;

void bm::server::matchmaking_service::process(
    const iscool::net::endpoint& endpoint, const iscool::net::message& message)
{
  assert(message.get_session_id() != 0);

  if (message.get_channel_id() != 0)
    return;

  if (message.get_type() == bm::net::message_type::new_game_request)
    create_or_update_game(endpoint, message.get_session_id(),
                          bm::net::new_game_request(message.get_content()));
  else if (message.get_type() == bm::net::message_type::accept_game)
    mark_as_ready(endpoint, message.get_session_id(),
                  bm::net::accept_game(message.get_content()));
}

void bm::server::matchmaking_service::create_or_update_game(
    const iscool::net::endpoint& endpoint,
    const iscool::net::session_id session,
    const bm::net::new_game_request& request)
{
  ic_causeless_log(iscool::log::nature::info(), "matchmaking_service",
                   "New game request for session %d.", session);

  char name[std::tuple_size_v<bm::net::game_name> + 1];
  std::copy(request.get_name().begin(), request.get_name().end(), name);
  name[std::size(name) - 1] = '\0';

  const name_to_game_id_map::iterator it = m_game_ids.find(name);

  if (it == m_game_ids.end())
    create_game(endpoint, session, request, name);
  else
    update_game(endpoint, session, request, it->second, m_games[it->second]);
}

void bm::server::matchmaking_service::create_game(
    const iscool::net::endpoint& endpoint,
    const iscool::net::session_id session,
    const bm::net::new_game_request& request, std::string name)
{
  ic_causeless_log(iscool::log::nature::info(), "matchmaking_service",
                   "Creating new game %d ('%s') on request of session %d.",
                   m_next_game_id, name, session);

  const bm::net::game_id game_id
      = m_game_ids.emplace(name, m_next_game_id).first->second;
  ++m_next_game_id;

  m_game_names[game_id] = std::move(name);

  game_info& game = m_games[game_id];

  game.player_count = 1;
  game.sessions[0] = session;
  game.release_at_this_date[0] = date_for_next_release();
  game.clean_up_connection = schedule_clean_up(game_id);

  send_game_on_hold(endpoint, request.get_request_token(), session, game_id,
                    game.player_count);
}

void bm::server::matchmaking_service::update_game(
    const iscool::net::endpoint& endpoint,
    const iscool::net::session_id session,
    const bm::net::new_game_request& request, bm::net::game_id game_id,
    game_info& game)
{
  ic_causeless_log(
      iscool::log::nature::info(), "matchmaking_service",
      "Refreshing game '%d' with %d players on request of session %d.",
      game_id, (int)game.player_count, session);

  const std::size_t existing_index = game.session_index(session);

  // Update for a player on hold.
  if (existing_index < game.sessions.size())
    {
      game.release_at_this_date[existing_index] = date_for_next_release();
      remove_inactive_sessions(game_id, game);
    }
  else
    {
      remove_inactive_sessions(game_id, game);

      if (game.player_count != game.sessions.size())
        {
          const std::size_t new_index = game.player_count;
          game.sessions[new_index] = session;
          game.release_at_this_date[new_index] = date_for_next_release();
          ++game.player_count;
        }
      else
        return;
    }

  if (game.player_count == 0)
    clean_up(game_id);
  else
    {
      game.clean_up_connection = schedule_clean_up(game_id);
      send_game_on_hold(endpoint, request.get_request_token(), session,
                        game_id, game.player_count);
    }
}

void bm::server::matchmaking_service::send_game_on_hold(
    const iscool::net::endpoint& endpoint, bm::net::client_token token,
    iscool::net::session_id session, bm::net::game_id game_id,
    std::uint8_t player_count)
{
  m_message_stream.send(
      endpoint,
      bm::net::game_on_hold(token, game_id, player_count).build_message(),
      session, 0);
}

void bm::server::matchmaking_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bm::net::accept_game& message)
{
  const bm::net::game_id game_id = message.get_game_id();

  ic_causeless_log(iscool::log::nature::info(), "matchmaking_service",
                   "Player ready. Session %d, game %d.", session, game_id);

  assert(m_games.find(game_id) != m_games.end());
  game_info& game = m_games[game_id];
  const std::size_t existing_index = game.session_index(session);

  // Update for a player on hold.
  if (existing_index == game.sessions.size())
    {
      ic_causeless_log(iscool::log::nature::info(), "matchmaking_service",
                       "Session %d is not part of game %d.", session, game_id);
      return;
    }

  game.release_at_this_date[existing_index] = date_for_next_release();
  game.ready[existing_index] = true;
  game.clean_up_connection = schedule_clean_up(game_id);

  remove_inactive_sessions(game_id, game);

  int ready_count = 0;
  for (int i = 0; i != game.player_count; ++i)
    ready_count += game.ready[i];

  if ((ready_count == game.player_count) && (ready_count > 1))
    m_message_stream.send(
        endpoint,
        bm::net::launch_game(message.get_request_token(), game.player_count, 0)
            .build_message(),
        session, 0);
}

void bm::server::matchmaking_service::remove_inactive_sessions(
    bm::net::game_id game_id, game_info& game)
{
  const std::chrono::nanoseconds now
      = iscool::time::now<std::chrono::nanoseconds>();

  for (int i = 0; i != game.player_count;)
    if (game.release_at_this_date[i] <= now)
      {
        ic_causeless_log(iscool::log::nature::info(), "matchmaking_service",
                         "Kicking %d from %d: timeout.", game.sessions[i],
                         game_id);
        game.erase(i);
      }
    else
      ++i;
}

iscool::signals::connection
bm::server::matchmaking_service::schedule_clean_up(bm::net::game_id game_id)
{
  return iscool::schedule::delayed_call(
      [this, game_id]() -> void
      {
        clean_up(game_id);
      },
      std::chrono::minutes(3));
}

void bm::server::matchmaking_service::clean_up(bm::net::game_id game_id)
{
  ic_causeless_log(iscool::log::nature::info(), "matchmaking_service",
                   "Cleaning up game %d.", game_id);

  const game_id_to_name_map::iterator it = m_game_names.find(game_id);

  m_game_ids.erase(it->second);
  m_game_names.erase(it);
  m_games.erase(game_id);
}
