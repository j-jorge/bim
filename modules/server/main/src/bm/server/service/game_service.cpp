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
#include <bm/server/service/game_service.hpp>

#include <bm/server/service/game_info.hpp>

#include <bm/net/message/ready.hpp>
#include <bm/net/message/start.hpp>

#include <iscool/log/causeless_log.h>
#include <iscool/log/nature/info.h>
#include <iscool/time/now.h>

#include <algorithm>

struct bm::server::game_service::game
{
  std::uint8_t player_count;
  std::array<iscool::net::session_id, 4> sessions;
  std::array<bool, 4> ready;

  std::size_t session_index(iscool::net::session_id session) const
  {
    return std::find(sessions.begin(), sessions.end(), session)
           - sessions.begin();
  }
};

bm::server::game_service::game_service(iscool::net::socket_stream& socket)
  : m_message_stream(socket)
  , m_next_game_channel(1)
{}

bm::server::game_service::~game_service() = default;

std::optional<bm::server::game_info>
bm::server::game_service::find_game(iscool::net::channel_id channel) const
{
  const game_map::const_iterator it = m_games.find(channel);

  if (it == m_games.end())
    return std::nullopt;

  return game_info{ .channel = channel,
                    .player_count = it->second.player_count,
                    .sessions = it->second.sessions };
}

bm::server::game_info bm::server::game_service::new_game(
    std::uint8_t player_count,
    const std::array<iscool::net::session_id, 4>& sessions)
{
  const iscool::net::channel_id channel = m_next_game_channel;
  ++m_next_game_channel;

  ic_causeless_log(iscool::log::nature::info(), "game_service",
                   "Creating new game %d for %d players.", channel,
                   (int)player_count);

  game& game = m_games[channel];

  game.player_count = player_count;
  game.sessions = sessions;
  game.ready.fill(false);

  return game_info{ .channel = channel,
                    .player_count = game.player_count,
                    .sessions = game.sessions };
}

void bm::server::game_service::process(const iscool::net::endpoint& endpoint,
                                       const iscool::net::message& message)
{
  assert(message.get_session_id() != 0);

  const iscool::net::channel_id channel = message.get_channel_id();
  const game_map::iterator it = m_games.find(channel);

  if (it == m_games.end())
    {
      ic_causeless_log(iscool::log::nature::info(), "game_service",
                       "Game with channel %d does not exist.", channel);
      return;
    }

  if (message.get_type() == bm::net::message_type::ready)
    mark_as_ready(endpoint, message.get_session_id(), channel, it->second);
}

void bm::server::game_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel, game& game)
{
  const std::size_t existing_index = game.session_index(session);

  // Update for a player on hold.
  if (existing_index == game.sessions.size())
    {
      ic_causeless_log(iscool::log::nature::info(), "game_service",
                       "Session %d is not part of game %d.", session, channel);
      return;
    }

  game.ready[existing_index] = true;

  int ready_count = 0;
  for (int i = 0; i != game.player_count; ++i)
    ready_count += game.ready[i];

  if (ready_count != game.player_count)
    {
      ic_causeless_log(iscool::log::nature::info(), "game_service",
                       "Session %d ready %d/%d.", session, ready_count,
                       (int)game.player_count);
      return;
    }

  ic_causeless_log(iscool::log::nature::info(), "game_service",
                   "Channel %d all players ready.", channel);

  m_message_stream.send(endpoint, bm::net::start().build_message(), session,
                        channel);
}
