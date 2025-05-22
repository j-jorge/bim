// SPDX-License-Identifier: AGPL-3.0-only
#include "bim/server/service/server_stats.hpp"
#include <bim/server/service/game_service.hpp>

#include <bim/server/config.hpp>
#include <bim/server/service/contest_timeline_service.hpp>
#include <bim/server/service/game_info.hpp>

#include <bim/net/message/game_over.hpp>
#include <bim/net/message/game_update_from_client.hpp>
#include <bim/net/message/game_update_from_server.hpp>
#include <bim/net/message/ready.hpp>
#include <bim/net/message/start.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/kick_event.hpp>
#include <bim/game/player_action.hpp>

#include <bim/assume.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

#include <algorithm>
#include <array>
#include <limits>

static constexpr std::uint8_t g_brick_wall_probability = 80;

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
       bim::game::feature_flags features,
       const std::array<iscool::net::session_id,
                        bim::game::g_max_player_count>& sessions)
    : seed(seed)
    , features(features)
    , player_count(player_count)
    , sessions(sessions)
    , simulation_tick(0)
    , completed_tick_count_all(0)
    , contest_result(bim::game::contest_result::create_still_running())
    , contest({ .seed = seed,
                .features = features,
                .player_count = player_count,
                .brick_wall_probability = g_brick_wall_probability,
                .arena_width = bim::game::g_default_arena_width,
                .arena_height = bim::game::g_default_arena_height })
  {
    ready.fill(false);
    active.fill(false);
    completed_tick_count_per_player.fill(0);

    for (int i = 0; i != player_count; ++i)
      actions[i].reserve(32);
  }

  bim::game::contest_fingerprint contest_fingerprint() const
  {
    return { .seed = seed,
             .features = features,
             .player_count = player_count,
             .brick_wall_probability = g_brick_wall_probability,
             .arena_width = contest.arena().width(),
             .arena_height = contest.arena().height() };
  }

  std::size_t session_index(iscool::net::session_id session) const
  {
    return std::find(sessions.begin(), sessions.end(), session)
           - sessions.begin();
  }

  /**
   * Remove from the action list all actions that have been confirmed by all
   * players. This updates the globally-confirmed state of the simulation.
   *
   * \return True if the simulation has been updated, false otherwise.
   */
  bool drop_old_actions()
  {
    std::size_t offset = std::numeric_limits<std::size_t>::max();

    bim_assume(player_count >= 2);
    bim_assume(player_count <= bim::game::g_max_player_count);

    // Compute the number of actions to remove: largest common interval to all
    // players.
    for (std::uint8_t player_index = 0; player_index != player_count;
         ++player_index)
      if (active[player_index])
        {
          const std::size_t player_offset =
              completed_tick_count_per_player[player_index]
              - completed_tick_count_all;

          if (player_offset < offset)
            offset = player_offset;
        }

    if (offset == 0)
      return false;

    // Play the ticks reached by all players.
    seal_player_actions(offset);

    // Actually remove the actions.
    for (std::uint8_t player_index = 0; player_index != player_count;
         ++player_index)
      {
        std::vector<bim::game::player_action>& player_actions =
            actions[player_index];

        // Inactive players may not have "offset" actions, so we must restrict
        // the removal to what we actually have.
        player_actions.erase(player_actions.begin(),
                             player_actions.begin()
                                 + std::min(offset, player_actions.size()));
      }

    completed_tick_count_all += offset;

    return true;
  }

  /**
   * Update simulation_tick to the highest step confirmed by all players.
   */
  void align_simulation_tick()
  {
    std::uint32_t simulation_offset =
        std::numeric_limits<std::uint32_t>::max();

    bim_assume(player_count >= 2);
    bim_assume(player_count <= bim::game::g_max_player_count);

    for (std::uint8_t player_index = 0; player_index != player_count;
         ++player_index)
      if (active[player_index])
        {
          const std::uint32_t tick_count = actions[player_index].size();

          if (tick_count < simulation_offset)
            simulation_offset = tick_count;
        }

    simulation_tick = completed_tick_count_all + simulation_offset;
  }

private:
  void seal_player_actions(std::size_t count)
  {
    for (std::size_t i = 0; i != count; ++i)
      {
        std::array<bim::game::player_action*, bim::game::g_max_player_count>
            player_actions;
        bim::game::collect_player_actions(player_actions, contest.registry());

        for (int p = 0; p != player_count; ++p)
          if (player_actions[p])
            {
              if (i < actions[p].size())
                *player_actions[p] = actions[p][i];
              else if (i == actions[p].size())
                bim::game::kick_player(contest.registry(), p);
            }

        if (timeline_writer)
          timeline_writer.push(contest.registry());

        const bim::game::contest_result tick_result = contest.tick();

        if (contest_result.still_running() && !tick_result.still_running())
          {
            contest_result = tick_result;
            game_over_tick = completed_tick_count_all + i;
            timeline_writer = {};
          }
      }
  }

public:
  std::uint64_t seed;
  bim::game::feature_flags features;
  std::uint8_t player_count;
  std::array<iscool::net::session_id, bim::game::g_max_player_count> sessions;
  std::array<bool, bim::game::g_max_player_count> ready;
  std::array<bool, bim::game::g_max_player_count> active;

  /**
   * The tick reached by every player. At this point, the clients may not know
   * the state of every other player, but every client has simulated up to this
   * point. This is the minimum of (completed_tick_count_per_player[i] +
   * actions[i].size()).
   */
  std::uint32_t simulation_tick;

  /**
   * The tick confirmed by every client, i.e. each client knows the actions of
   * every player up to this point. This is the minimum of
   * completed_tick_count_per_player[i].
   */
  std::uint32_t completed_tick_count_all;

  /**
   * The tick of the simulation for every player. The player has applied the
   * actions from all players up to this point.
   */
  std::array<std::uint32_t, bim::game::g_max_player_count>
      completed_tick_count_per_player;

  /**
   * The actions to apply starting from completed_tick_count_per_player, for
   * each player. One player_action per tick.
   */
  std::array<std::vector<bim::game::player_action>,
             bim::game::g_max_player_count>
      actions;

  std::array<std::chrono::nanoseconds, bim::game::g_max_player_count>
      release_player_at_this_date;

  std::chrono::nanoseconds release_game_at_this_date;

  std::uint32_t game_over_tick;
  bim::game::contest_result contest_result;

  bim::game::contest contest;

  bim::game::contest_timeline_writer timeline_writer;
};

bim::server::game_service::game_service(const config& config,
                                        iscool::net::socket_stream& socket,
                                        server_stats& stats)
  : m_server_stats(stats)
  , m_message_stream(socket)
  , m_next_game_channel(1)
  , m_random(config.random_seed)
  , m_clean_up_interval(config.game_service_clean_up_interval)
  , m_disconnection_lateness_threshold_in_ticks(
        config.game_service_disconnection_lateness_threshold_in_ticks)
  , m_disconnection_earliness_threshold_in_ticks(
        config.game_service_disconnection_earliness_threshold_in_ticks)
  , m_disconnection_inactivity_delay(
        config.game_service_disconnection_inactivity_delay)
  , m_message_pool(64)
{
  if (config.enable_contest_timeline_recording)
    m_contest_timeline_service.reset(new contest_timeline_service(config));

  schedule_clean_up();
}

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

  return (it != m_games.end()) && it->second.contest_result.still_running();
}

std::optional<bim::server::game_info>
bim::server::game_service::find_game(iscool::net::channel_id channel) const
{
  const game_map::const_iterator it = m_games.find(channel);

  if (it == m_games.end())
    return std::nullopt;

  return game_info{ .fingerprint = it->second.contest_fingerprint(),
                    .channel = channel,
                    .sessions = it->second.sessions };
}

bim::server::game_info bim::server::game_service::new_game(
    std::uint8_t player_count, bim::game::feature_flags features,
    const std::array<iscool::net::session_id, bim::game::g_max_player_count>&
        sessions)
{
  const iscool::net::channel_id channel = m_next_game_channel;
  ++m_next_game_channel;

  ic_log(iscool::log::nature::info(), "game_service",
         "Creating new game {} for {} players.", channel, (int)player_count);

  game& game =
      m_games
          .emplace(std::piecewise_construct, std::forward_as_tuple(channel),
                   std::forward_as_tuple(player_count, m_random(), features,
                                         sessions))
          .first->second;

  for (int i = 0; i != player_count; ++i)
    m_session_to_channel[sessions[i]] = channel;

  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();

  game.release_game_at_this_date = now + m_clean_up_interval;

  for (int i = 0; i != player_count; ++i)
    {
      game.active[i] = true;
      game.release_player_at_this_date[i] =
          now + m_disconnection_inactivity_delay;
    }

  const bim::game::contest_fingerprint contest_fingerprint =
      game.contest_fingerprint();

  if (m_contest_timeline_service)
    game.timeline_writer =
        m_contest_timeline_service->open(channel, contest_fingerprint);

  m_server_stats.record_game_start(player_count);

  return game_info{ .fingerprint = contest_fingerprint,
                    .channel = channel,
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
             "Game with channel {} does not exist.", channel);
      return;
    }

  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();

  it->second.release_game_at_this_date = now + m_clean_up_interval;

  switch (message.get_type())
    {
    case bim::net::message_type::ready:
      mark_as_ready(endpoint, message.get_session_id(), channel, it->second,
                    now);
      break;
    case bim::net::message_type::game_update_from_client:
      push_update(endpoint, channel, message, it->second, now);
      break;
    }
}

void bim::server::game_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel, game& game, std::chrono::nanoseconds now)
{
  const std::size_t existing_index = game.session_index(session);

  // Update for a player on hold.
  if (existing_index == game.sessions.size())
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Session {} is not part of game {}.", session, channel);
      return;
    }

  if (!game.active[existing_index])
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Session {} has been kicked from game {}.", session, channel);
      return;
    }

  game.ready[existing_index] = true;
  game.active[existing_index] = true;

  game.release_player_at_this_date[existing_index] =
      now + m_disconnection_inactivity_delay;

  check_drop_desynchronized_player(channel, game, now);

  int ready_count = 0;
  int active_count = 0;

  for (int i = 0; i != game.player_count; ++i)
    {
      ready_count += game.ready[i];
      active_count += game.active[i];
    }

  if (ready_count != active_count)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Session {} ready {}/{}.", session, ready_count,
             (int)game.player_count);
      return;
    }

  ic_log(iscool::log::nature::info(), "game_service",
         "Channel {} all players ready, session {}.", channel, session);

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::start().build_message(*s.value);

  m_message_stream.send(endpoint, *s.value, session, channel);
  m_message_pool.release(s.id);
}

void bim::server::game_service::push_update(
    const iscool::net::endpoint& endpoint, iscool::net::channel_id channel,
    const iscool::net::message& message, game& game,
    std::chrono::nanoseconds now)
{
  const std::optional<bim::net::game_update_from_client> update =
      bim::net::try_deserialize_message<bim::net::game_update_from_client>(
          message);
  const iscool::net::session_id session = message.get_session_id();

  if (!update)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Could not deserialize message game update from session={}.",
             session);
      return;
    }

  const std::size_t player_index = game.session_index(session);
  const std::optional<std::size_t> tick_count =
      validate_message(*update, session, player_index, game);

  if (!tick_count)
    return;

  game.release_player_at_this_date[player_index] =
      now + m_disconnection_inactivity_delay;

  if (*tick_count != 0)
    queue_actions(*update, player_index, game);

  const bool simulation_updated = game.drop_old_actions();
  game.align_simulation_tick();

  if (!simulation_updated)
    check_drop_desynchronized_player(channel, game, now);

  if (!game.contest_result.still_running()
      && (game.game_over_tick
          <= game.completed_tick_count_per_player[player_index]))
    send_game_over(endpoint, session, channel, game);
  else
    send_actions(endpoint, session, channel, player_index, game);
}

/// Validate the integrity of the message.
std::optional<std::size_t> bim::server::game_service::validate_message(
    const bim::net::game_update_from_client& message,
    iscool::net::session_id session, std::size_t player_index,
    const game& game) const
{
  if (player_index >= game.player_count)
    return std::nullopt;

  // The player has been disconnected on the server's decision.
  if (!game.active[player_index])
    return std::nullopt;

  // A message from the player's past, maybe it took a longer path on the
  // network.
  if (message.from_tick < game.completed_tick_count_per_player[player_index])
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Message from the past from session={}, player={}, got "
             "{}, expected {}.",
             session, (int)player_index, message.from_tick,
             game.completed_tick_count_per_player[player_index]);
      return std::nullopt;
    }

  // A message from our future? Should not happen.
  if (message.from_tick > game.simulation_tick)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Message from the future from session={}, player={}, "
             "got {}, expected {}.",
             session, (int)player_index, message.from_tick,
             game.simulation_tick);
      return std::nullopt;
    }

  const std::size_t tick_count = message.actions.size();
  const std::size_t player_tick =
      game.completed_tick_count_all + game.actions[player_index].size();

  // A range of actions that happened before the player's current simulation
  // step, maybe it was lost on the network, or the player has not received our
  // update.
  if (message.from_tick + tick_count <= player_tick)
    {
      // This happens quite frequently, so no log for it.
      return 0;
    }

  if (message.from_tick > player_tick)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Gap in player message for session={}, player={}, (received) {} "
             "> {} (bound).",
             session, (int)player_index, message.from_tick, tick_count,
             player_tick);
      return 0;
    }

  if (tick_count > 255)
    {
      ic_log(iscool::log::nature::info(), "game_service",
             "Too many action count/ticks {} for session={}, player={}.",
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
    iscool::net::channel_id channel, std::size_t player_index,
    const game& game)
{
  // All we need to send to a player P is the sequence of actions from the
  // other players that happened between the last synchronized tick of P and
  // the smallest predicted tick reached by all the players.

  const std::uint32_t completed_tick_count_for_player =
      game.completed_tick_count_per_player[player_index];

  // The player may be farther than the point confirmed by all players (e.g.
  // some players are late in the synchronization).
  bim_assume(completed_tick_count_for_player >= game.completed_tick_count_all);
  const std::size_t tick_start_index =
      completed_tick_count_for_player - game.completed_tick_count_all;

  // The number of ticks that have been predicted by all players and that are
  // after the synchronized tick of this player.
  bim_assume(game.simulation_tick >= completed_tick_count_for_player);
  const std::size_t tick_count = std::min<std::size_t>(
      255, game.simulation_tick - completed_tick_count_for_player);

  bim::net::game_update_from_server message;
  message.from_tick = completed_tick_count_for_player;
  message.actions.resize(game.player_count);

  for (std::uint8_t player = 0; player != game.player_count; ++player)
    {
      // We must send as many actions as possible for inactive players because
      // we don't know how far the other players had applied its actions
      // before it was kicked out.
      //
      // As an example, let p1 be a kicked out player. When it was kicked out
      // its simulation was at tick 100, simulation_tick was 90 (i.e. all
      // players where at least at tick 90), completed_tick_count_all was 70
      // (i.e. all clients knew the state of every other client up to tick
      // 70). The list of actions for p1 had thus 30 actions.
      //
      // Later, simulation_tick is 105 but completed_tick_count_all is 80. We
      // are sending a message to p0, and we have
      // completed_tick_count_per_player[p0] >= 80. Let's say its'
      // 80. Consequently we must send to p0 all actions from 80 to 105 for p0,
      // p2, and p3, and from 80 to 100 for p1.
      const std::size_t action_count = game.actions[player].size();
      if (tick_start_index >= action_count)
        continue;

      const std::size_t adjusted_tick_count =
          std::min(tick_count, action_count - tick_start_index);

      const std::vector<bim::game::player_action>::const_iterator begin =
          game.actions[player].begin() + tick_start_index;
      message.actions[player].insert(message.actions[player].end(), begin,
                                     begin + adjusted_tick_count);
    }

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  message.build_message(*s.value);

  m_message_stream.send(endpoint, *s.value, session, channel);
  m_message_pool.release(s.id);
}

void bim::server::game_service::send_game_over(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    iscool::net::channel_id channel, const game& game)
{
  assert(!game.contest_result.still_running());

  const int winner_index = game.contest_result.has_a_winner()
                               ? game.contest_result.winning_player()
                               : bim::game::g_max_player_count;

  ic_log(iscool::log::nature::info(), "game_service",
         "Sending game over, session={}, game_id={}, winner={}.", session,
         channel, winner_index);

  const bim::net::game_over message(winner_index);
  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  message.build_message(*s.value);

  m_message_stream.send(endpoint, *s.value, session, channel);
  m_message_pool.release(s.id);
}

void bim::server::game_service::check_drop_desynchronized_player(
    iscool::net::channel_id channel, game& game,
    std::chrono::nanoseconds now) const
{
  // Exclude the players for which we have no news since a long time.
  for (int i = 0; i != game.player_count; ++i)
    if (game.active[i] && (now >= game.release_player_at_this_date[i]))
      {
        ic_log(iscool::log::nature::info(), "game_service",
               "Kicking out player={}, session={}, from game_id={}, due to "
               "lateness. No news since {}.",
               i, game.sessions[i], channel, m_disconnection_inactivity_delay);
        game.active[i] = false;
      }

  std::array<int, bim::game::g_max_player_count> player_tick;
  std::array<std::uint8_t, bim::game::g_max_player_count> player_index;
  int player_count = 0;

  for (int i = 0; i != game.player_count; ++i)
    if (game.active[i])
      {
        player_tick[player_count] =
            game.completed_tick_count_per_player[i] + game.actions[i].size();
        player_index[player_count] = i;
        ++player_count;

        for (int j = player_count - 1; j != 0; --j)
          if (player_tick[j] < player_tick[j - 1])
            {
              std::swap(player_tick[j], player_tick[j - 1]);
              std::swap(player_index[j], player_index[j - 1]);
            }
      }

  assert(
      std::is_sorted(player_tick.begin(), player_tick.begin() + player_count));

  if (player_count < 2)
    return;

  const int slowest_tick = player_tick[0];
  const int second_slowest_tick = player_tick[1];

  // Exclude the slowest player if it is too far behind.
  if (second_slowest_tick - slowest_tick
      >= m_disconnection_lateness_threshold_in_ticks)
    {
      const int i = player_index[0];
      ic_log(iscool::log::nature::info(), "game_service",
             "Kicking out player={}, session={}, from game_id={}, due to "
             "lateness. Tick is {}, {} behind {}. Threshold is {}.",
             i, game.sessions[i], channel, slowest_tick,
             second_slowest_tick - slowest_tick, second_slowest_tick,
             m_disconnection_lateness_threshold_in_ticks);
      game.active[i] = false;
      return;
    }

  // Exclude the fastest player if it is too far ahead. Aren't they waiting for
  // the server updates?
  const int fastest_tick = player_tick[player_count - 1];
  const int second_fastest_tick = player_tick[player_count - 2];

  if (fastest_tick - second_fastest_tick
      >= m_disconnection_earliness_threshold_in_ticks)
    {
      const int i = player_index[player_count - 1];
      ic_log(iscool::log::nature::info(), "game_service",
             "Kicking out player={}, session={}, from game_id={}, due to "
             "earliness. Tick is {}, {} ahead of {}. Threshold is {}.",
             i, game.sessions[i], channel, fastest_tick,
             fastest_tick - second_fastest_tick, second_fastest_tick,
             m_disconnection_earliness_threshold_in_ticks);
      game.active[i] = false;
    }
}

void bim::server::game_service::schedule_clean_up()
{
  m_clean_up_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        clean_up();
      },
      m_clean_up_interval);
}

void bim::server::game_service::clean_up()
{
  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();

  for (game_map::iterator it = m_games.begin(); it != m_games.end();)
    if (it->second.release_game_at_this_date <= now)
      {
        clean_up(it->first, it->second);
        it = m_games.erase(it);
      }
    else
      ++it;

  schedule_clean_up();
}

void bim::server::game_service::clean_up(iscool::net::channel_id channel,
                                         const game& g)
{
  ic_log(iscool::log::nature::info(), "game_service", "Cleaning up game {}.",
         channel);

  m_server_stats.record_game_end(g.player_count);

  const session_to_channel_map::const_iterator eit =
      m_session_to_channel.end();

  for (int i = 0; i != g.player_count; ++i)
    {
      // The sessions may have been assigned to another game between the end of
      // the game and the clean-up, for example if the player has asked for
      // another match. Consequently we must check the channel assigned to the
      // session before removing it from the map.
      const session_to_channel_map::iterator stc =
          m_session_to_channel.find(g.sessions[i]);

      if ((stc != eit) && (stc->second == channel))
        m_session_to_channel.erase(stc);
    }
}
