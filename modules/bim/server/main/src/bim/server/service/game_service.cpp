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
#include <bim/server/service/game_service.hpp>

#include <bim/server/service/game_info.hpp>

#include <bim/net/message/game_update_from_client.hpp>
#include <bim/net/message/game_update_from_server.hpp>
#include <bim/net/message/ready.hpp>
#include <bim/net/message/start.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_result.hpp>

#include <bim/assume.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

#include <algorithm>
#include <array>
#include <limits>

struct bim::server::game_service::game
{
  /*
    In short, the communication works as follows:
    - The clients send their actions,
    - The server answer with the actions of all clients.

    In order to send the actions of the other clients we need to keep a range
    of player actions for each player. When a client sends its updates, we know
    his reference point in the simulation, and the sequence of actions they
    executed (and thus the number of steps in the simulation). Their reference
    point in the simulation is based on completed_tick_count_per_player, the
    actions are stored in actions. When the game starts,
    completed_tick_count_per_player is zero for each player. Then, when a
    player sends the first tick from his side, completed_tick_count_per_player
    becomes 1 for him. In other words, completed_tick_count_per_player is the
    index of the next tick to be done for each player.

    We store in simulation_tick the step at which the simulation is currently
    paused. When every player has sent its actions for this step, we increment
    it. Consequently, the game updates at the pace of the slowest player.

    Actions are kept as long as at least one player has not confirmed the
    tick. We store the count of ticks that have been confirmed by all players
    (i.e. the index of the tick at which must be applied the first action in
    each of the vectors in actions below) in completed_tick_count_all.
  */

public:
  game(std::uint8_t player_count, std::uint64_t seed,
       const std::array<iscool::net::session_id, 4>& sessions)
    : seed(seed)
    , player_count(player_count)
    , sessions(sessions)
    , simulation_tick(0)
    , completed_tick_count_all(0)
    , m_contest(seed, 80, player_count, 11, 13)
  {
    ready.fill(false);
    completed_tick_count_per_player.fill(0);

    for (int i = 0; i != player_count; ++i)
      actions[i].reserve(32);

    m_contest.registry()
        .view<bim::game::player, bim::game::player_action>()
        .each(
            [this](const bim::game::player& player,
                   bim::game::player_action& action)
            {
              m_player_actions[player.index] = &action;
            });
  }

  std::size_t session_index(iscool::net::session_id session) const
  {
    return std::find(sessions.begin(), sessions.end(), session)
           - sessions.begin();
  }

  /**
   * Remove from the action list all actions that have been confirmed by all
   * players, i.e. everything between the first action and min_{i \in 0..3}
   * last_received_player_tick[i] included. Update first_stored_tick
   * accordingly.
   */
  void drop_old_actions()
  {
    std::uint32_t offset = std::numeric_limits<std::uint32_t>::max();

    bim_assume(player_count >= 2);
    bim_assume(player_count <= 4);

    // Compute the number of actions to remove: largest common interval to all
    // players.
    for (std::uint8_t player_index = 0; player_index != player_count;
         ++player_index)
      {
        const std::uint32_t player_offset =
            completed_tick_count_per_player[player_index]
            - completed_tick_count_all;

        if (player_offset < offset)
          offset = player_offset;
      }

    if (offset == 0)
      return;

    // Play the ticks reached by all players.
    seal_player_actions(offset);

    // Actually remove the actions.
    for (std::uint8_t player_index = 0; player_index != player_count;
         ++player_index)
      actions[player_index].erase(actions[player_index].begin(),
                                  actions[player_index].begin() + offset);

    completed_tick_count_all += offset;
  }

  /**
   * Update simulation_tick to the highest step confirmed by all players.
   */
  void align_simulation_tick()
  {
    std::uint32_t simulation_offset =
        std::numeric_limits<std::uint32_t>::max();

    bim_assume(player_count >= 2);
    bim_assume(player_count <= 4);

    for (std::uint8_t player_index = 0; player_index != player_count;
         ++player_index)
      {
        const std::uint32_t tick_count = actions[player_index].size();

        if (tick_count < simulation_offset)
          simulation_offset = tick_count;
      }

    simulation_tick = completed_tick_count_all + simulation_offset;
  }

private:
  void seal_player_actions(int count)
  {
    bim::game::contest_result result;

    for (int i = 0; i != count; ++i)
      {
        for (int p = 0; p != player_count; ++p)
          *m_player_actions[p] = actions[p][i];

        result = m_contest.tick();
      }

    game_over = !result.still_running();
  }

public:
  std::uint64_t seed;
  std::uint8_t player_count;
  std::array<iscool::net::session_id, 4> sessions;
  std::array<bool, 4> ready;

  /**
   * The tick reached by every player. At this point, the clients may not know
   * the state of every other player, but every client as simulated up to this
   * point. This is the minimum of (completed_tick_count_per_player[i] +
   * actions[i].size()).
   */
  std::uint32_t simulation_tick;

  /**
   * The tick confirmed by every client, i.e. each client knows the actions of
   * every player up to this point.
   */
  std::uint32_t completed_tick_count_all;

  /// The tick of the simulation for every player.
  std::array<std::uint32_t, 4> completed_tick_count_per_player;

  /**
   * The actions to apply starting from completed_tick_count_per_player, for
   * each player. One player_action per tick.
   */
  std::array<std::vector<bim::game::player_action>, 4> actions;

  bool game_over;

private:
  bim::game::contest m_contest;
  std::array<bim::game::player_action*, 4> m_player_actions;
};

bim::server::game_service::game_service(iscool::net::socket_stream& socket)
  : m_message_stream(socket)
  , m_next_game_channel(1)
  , m_random(std::random_device()())
{}

bim::server::game_service::~game_service() = default;

bool bim::server::game_service::is_in_active_game(
    iscool::net::session_id session) const
{
  const session_to_channel_map::const_iterator it =
      m_session_to_channel.find(session);

  return (it != m_session_to_channel.end()) && is_playing(it->second);
}

bool bim::server::game_service::is_playing(
    iscool::net::channel_id channel) const
{
  const game_map::const_iterator it = m_games.find(channel);

  return (it != m_games.end()) && !it->second.game_over;
}

std::optional<bim::server::game_info>
bim::server::game_service::find_game(iscool::net::channel_id channel) const
{
  const game_map::const_iterator it = m_games.find(channel);

  if (it == m_games.end())
    return std::nullopt;

  return game_info{ .seed = it->second.seed,
                    .channel = channel,
                    .player_count = it->second.player_count,
                    .sessions = it->second.sessions };
}

bim::server::game_info bim::server::game_service::new_game(
    std::uint8_t player_count,
    const std::array<iscool::net::session_id, 4>& sessions)
{
  const iscool::net::channel_id channel = m_next_game_channel;
  ++m_next_game_channel;

  ic_log(iscool::log::nature::info(), "game_service",
         "Creating new game %d for %d players.", channel, (int)player_count);

  game& game =
      m_games
          .emplace(std::piecewise_construct, std::forward_as_tuple(channel),
                   std::forward_as_tuple(player_count, m_random(), sessions))
          .first->second;

  return game_info{ .seed = game.seed,
                    .channel = channel,
                    .player_count = game.player_count,
                    .sessions = game.sessions };
}

void bim::server::game_service::process(const iscool::net::endpoint& endpoint,
                                        const iscool::net::message& message)
{
  assert(message.get_session_id() != 0);

  const iscool::net::channel_id channel = message.get_channel_id();
  const game_map::iterator it = m_games.find(channel);

  if (it == m_games.end())
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Game with channel %d does not exist.", channel);
      return;
    }

  switch (message.get_type())
    {
    case bim::net::message_type::ready:
      mark_as_ready(endpoint, message.get_session_id(), channel, it->second);
      break;
    case bim::net::message_type::game_update_from_client:
      push_update(endpoint, channel, message, it->second);
      break;
    }
}

void bim::server::game_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel, game& game)
{
  const std::size_t existing_index = game.session_index(session);

  // Update for a player on hold.
  if (existing_index == game.sessions.size())
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Session %d is not part of game %d.", session, channel);
      return;
    }

  game.ready[existing_index] = true;

  int ready_count = 0;
  for (int i = 0; i != game.player_count; ++i)
    ready_count += game.ready[i];

  if (ready_count != game.player_count)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Session %d ready %d/%d.", session, ready_count,
             (int)game.player_count);
      return;
    }

  ic_log(iscool::log::nature::info(), "game_service",
         "Channel %d all players ready, session %d.", channel, session);

  m_message_stream.send(endpoint, bim::net::start().build_message(), session,
                        channel);
}

void bim::server::game_service::push_update(
    const iscool::net::endpoint& endpoint, iscool::net::channel_id channel,
    const iscool::net::message& message, game& game)
{
  const std::optional<bim::net::game_update_from_client> update =
      bim::net::try_deserialize_message<bim::net::game_update_from_client>(
          message);
  const iscool::net::session_id session = message.get_session_id();

  if (!update)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Could not deserialize message game update from session=%d.",
             session);
      return;
    }

  const std::size_t player_index = game.session_index(session);
  const std::optional<std::size_t> tick_count =
      validate_message(*update, session, player_index, game);

  if (!tick_count)
    return;

  if (*tick_count != 0)
    queue_actions(*update, player_index, game);

  send_actions(endpoint, session, channel, player_index, game);
  game.drop_old_actions();
  game.align_simulation_tick();
}

/// Validate the integrity of the message.
std::optional<std::size_t> bim::server::game_service::validate_message(
    const bim::net::game_update_from_client& message,
    iscool::net::session_id session, std::size_t player_index,
    const game& game) const
{
  if (player_index >= game.player_count)
    return std::nullopt;

  // A message from the player's past, maybe it took a longer path on the
  // network.
  if (message.from_tick < game.completed_tick_count_per_player[player_index])
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Message from the past from session=%d, player=%d, got "
             "%d, expected %d.",
             session, (int)player_index, message.from_tick,
             game.completed_tick_count_per_player[player_index]);
      return std::nullopt;
    }

  // A message from our future? Should not happen.
  if (message.from_tick > game.simulation_tick)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Message from the future from session=%d, player=%d, "
             "got %d, expected %d.",
             session, (int)player_index, message.from_tick,
             game.simulation_tick);
      return std::nullopt;
    }
  const std::size_t tick_count = message.actions.size();

  // A range of actions that happened before the current simulation step, maybe
  // it was lost on the network, or the player has not received our update.
  if (message.from_tick + tick_count <= game.simulation_tick)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Out-of-date message for session=%d, player=%d, got"
             " %d+%d=%d, expected %d.",
             session, (int)player_index, message.from_tick, tick_count,
             message.from_tick + tick_count, game.simulation_tick + 1);
      return 0;
    }

  if (tick_count > 255)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Too many action count/ticks %d for session=%d, player=%d.",
             tick_count, session, (int)player_index);
      return std::nullopt;
    }

  return tick_count;
}

void bim::server::game_service::queue_actions(
    const bim::net::game_update_from_client& message, std::size_t player_index,
    game& game)
{
  std::vector<bim::game::player_action>& local_actions =
      game.actions[player_index];

  const std::size_t tick_count = message.actions.size();

  std::size_t tick_index =
      game.completed_tick_count_all + local_actions.size() - message.from_tick;
  bim_assume(tick_index <= tick_count);

  game.completed_tick_count_per_player[player_index] = message.from_tick;

  // Apply the remaining actions.
  local_actions.insert(local_actions.end(),
                       message.actions.begin() + tick_index,
                       message.actions.end());
}

void bim::server::game_service::send_actions(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel, std::size_t player_index, game& game)
{
  const std::uint32_t last_received_player_tick =
      game.completed_tick_count_per_player[player_index];

  bim_assume(last_received_player_tick >= game.completed_tick_count_all);
  const std::size_t tick_start_index =
      last_received_player_tick - game.completed_tick_count_all;

  bim_assume(game.simulation_tick >= last_received_player_tick);
  const std::size_t tick_count = std::min<std::size_t>(
      255, game.simulation_tick - last_received_player_tick);

  bim::net::game_update_from_server message;
  message.from_tick = last_received_player_tick;
  message.actions.resize(game.player_count);

  for (std::uint8_t player = 0; player != game.player_count; ++player)
    {
      const std::vector<bim::game::player_action>::const_iterator begin =
          game.actions[player].begin() + tick_start_index;
      message.actions[player].insert(message.actions[player].end(), begin,
                                     begin + tick_count);
    }

  m_message_stream.send(endpoint, message.build_message(), session, channel);
}
