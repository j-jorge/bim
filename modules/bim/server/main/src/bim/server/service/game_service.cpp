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

#include <bim/game/assume.hpp>
#include <bim/game/component/player_action.hpp>

#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/info.hpp>

#include <algorithm>
#include <array>
#include <limits>

struct bim::server::game_service::game
{
  std::uint8_t player_count;
  std::array<iscool::net::session_id, 4> sessions;
  std::array<bool, 4> ready;

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
  std::uint32_t simulation_tick;
  std::uint32_t completed_tick_count_all;
  std::array<std::uint32_t, 4> completed_tick_count_per_player;
  std::array<std::vector<bim::game::player_action>, 4> actions;

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
};

bim::server::game_service::game_service(iscool::net::socket_stream& socket)
  : m_message_stream(socket)
  , m_next_game_channel(1)
{}

bim::server::game_service::~game_service() = default;

std::optional<bim::server::game_info>
bim::server::game_service::find_game(iscool::net::channel_id channel) const
{
  const game_map::const_iterator it = m_games.find(channel);

  if (it == m_games.end())
    return std::nullopt;

  return game_info{ .channel = channel,
                    .player_count = it->second.player_count,
                    .sessions = it->second.sessions };
}

bim::server::game_info bim::server::game_service::new_game(
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
  game.simulation_tick = 0;
  game.completed_tick_count_all = 0;
  game.completed_tick_count_per_player.fill(0);

  for (std::vector<bim::game::player_action>& v : game.actions)
    v.reserve(32);

  return game_info{ .channel = channel,
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
      ic_causeless_log(iscool::log::nature::info(), "game_service",
                       "Game with channel %d does not exist.", channel);
      return;
    }

  switch (message.get_type())
    {
    case bim::net::message_type::ready:
      mark_as_ready(endpoint, message.get_session_id(), channel, it->second);
      break;
    case bim::net::message_type::game_update_from_client:
      push_update(endpoint, message.get_session_id(), channel,
                  bim::net::game_update_from_client(message.get_content()),
                  it->second);
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
                   "Channel %d all players ready, session %d.", channel,
                   session);

  m_message_stream.send(endpoint, bim::net::start().build_message(), session,
                        channel);
}

void bim::server::game_service::push_update(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel,
    const bim::net::game_update_from_client& message, game& game)
{
  const std::size_t player_index = game.session_index(session);
  const std::optional<std::size_t> tick_count =
      validate_message(message, player_index, game);

  if (!tick_count)
    return;

  if (*tick_count != 0)
    queue_actions(message, player_index, game);

  send_actions(endpoint, session, channel, player_index, game);
  game.drop_old_actions();
  game.align_simulation_tick();
}

/// Validate the integrity of the message.
std::optional<std::size_t> bim::server::game_service::validate_message(
    const bim::net::game_update_from_client& message, std::size_t player_index,
    const game& game) const
{
  if (player_index >= game.player_count)
    return std::nullopt;

  // A message from the player's past, maybe it took a longer path on the
  // network.
  if (message.from_tick < game.completed_tick_count_per_player[player_index])
    return std::nullopt;

  // A message from our future? Should not happen.
  if (message.from_tick > game.simulation_tick)
    return std::nullopt;

  const std::size_t tick_count = message.action_count_at_tick.size();

  // A range of actions that happened before the current simulation step, maybe
  // it was lost on the network, or the player has not received our update.
  if (message.from_tick + tick_count <= game.simulation_tick)
    return 0;

  if (message.action_count_at_tick.size() > 255)
    return std::nullopt;

  if (message.actions.size() > bim::game::player_action::queue_capacity * 255)
    return std::nullopt;

  std::size_t action_count = 0;

  for (std::uint8_t c : message.action_count_at_tick)
    action_count += c;

  if (action_count != message.actions.size())
    return std::nullopt;

  return tick_count;
}

void bim::server::game_service::queue_actions(
    const bim::net::game_update_from_client& message, std::size_t player_index,
    game& game)
{
  std::size_t tick_index = game.completed_tick_count_all
                           + game.actions[player_index].size()
                           - message.from_tick;
  bim_assume(tick_index <= message.action_count_at_tick.size());

  std::size_t action_index = 0;

  // Skip the actions we have already seen.
  for (std::size_t i = 0; i != tick_index; ++i)
    action_index += message.action_count_at_tick[i];

  const std::size_t tick_count = message.action_count_at_tick.size();

  game.completed_tick_count_per_player[player_index] = message.from_tick;
  game.actions[player_index].reserve(game.actions[player_index].size()
                                     + tick_count - tick_index);

  // Apply the remaining actions.
  for (; tick_index != tick_count; ++tick_index)
    {
      const std::uint8_t action_count =
          message.action_count_at_tick[tick_index];
      bim_assume(action_count <= bim::game::player_action::queue_capacity);

      bim::game::player_action& action =
          game.actions[player_index].emplace_back();
      action.queue_size = action_count;

      std::copy_n(message.actions.begin() + action_index, action_count,
                  action.queue);
      action_index += action_count;
    }
}

void bim::server::game_service::send_actions(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel, std::size_t player_index, game& game)
{
  const std::uint32_t last_received_player_tick =
      game.completed_tick_count_per_player[player_index];

  bim_assume(last_received_player_tick >= game.completed_tick_count_all);
  const std::size_t action_start_index =
      last_received_player_tick - game.completed_tick_count_all;

  bim_assume(game.simulation_tick >= last_received_player_tick);
  const std::size_t action_count =
      game.simulation_tick - last_received_player_tick;

  const std::size_t action_end_index = action_start_index + action_count;

  bim::net::game_update_from_server message;
  message.first_tick = last_received_player_tick;
  message.action_count.reserve(action_count * game.player_count);
  // Times 2 because we expect at fewer than two action kinds per frame per
  // player.
  message.actions.reserve(action_count * game.player_count * 2);

  for (std::size_t action_index = action_start_index;
       action_index != action_end_index; ++action_index)
    {
      for (std::uint8_t player = 0; player != game.player_count; ++player)
        {
          assert(action_start_index <= game.actions[player].size());
          assert(action_end_index <= game.actions[player].size());
          bim_assume(action_index <= game.actions[player].size());

          const bim::game::player_action& action =
              game.actions[player][action_index];

          message.action_count.emplace_back(action.queue_size);

          for (int i = 0; i != action.queue_size; ++i)
            message.actions.emplace_back(action.queue[i]);
        }

      if (message.message_size() >= 480)
        break;
    }

  m_message_stream.send(endpoint, message.build_message(), session, channel);
}
