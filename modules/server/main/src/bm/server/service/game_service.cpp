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

#include <iscool/log/causeless_log.h>
#include <iscool/log/nature/info.h>
#include <iscool/time/now.h>

struct bm::server::game_service::game
{
  std::uint8_t player_count;
  std::array<iscool::net::session_id, 4> sessions;
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
  ic_causeless_log(iscool::log::nature::info(), "game_service",
                   "Creating new game for %d players.", (int)player_count);

  const iscool::net::channel_id channel = m_next_game_channel;
  ++m_next_game_channel;

  game& game = m_games[channel];

  game.player_count = player_count;
  game.sessions = sessions;

  return game_info{ .channel = channel,
                    .player_count = game.player_count,
                    .sessions = game.sessions };
}
