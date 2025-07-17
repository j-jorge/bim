// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/contest_runner.hpp>

#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/exchange/server_update.hpp>

#include <bim/game/check_game_over.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/arena_reduction_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/game_timer.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/input_archive.hpp>
#include <bim/game/kick_event.hpp>
#include <bim/game/output_archive.hpp>
#include <bim/game/player_action.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

bim::net::contest_runner::contest_runner(bim::game::contest& contest,
                                         game_update_exchange& update_exchange,
                                         std::uint8_t local_player_index,
                                         std::uint8_t player_count)
  : m_local_player_index(local_player_index)
  , m_player_count(player_count)
  , m_contest(contest)
  , m_update_exchange(update_exchange)
  , m_last_confirmed_tick(0)
  , m_last_completed_tick(0)
{
  for (std::vector<bim::game::player_action>& server_actions :
       m_server_actions)
    server_actions.reserve(64);

  save_contest_state(contest.registry());

  for (int player_index = 0; player_index != player_count; ++player_index)
    if (player_index != local_player_index)
      m_unconfirmed_actions[player_index].emplace_back();

  update_exchange.connect_to_updated(
      std::bind(&contest_runner::queue_updates, this, std::placeholders::_1));

  update_exchange.connect_to_game_over(
      [this](const bim::game::contest_result& result) -> void
      {
        m_contest_result = result;
      });
}

std::uint32_t bim::net::contest_runner::local_tick() const
{
  return m_last_completed_tick;
}

std::uint32_t bim::net::contest_runner::confirmed_tick() const
{
  return m_last_confirmed_tick;
}

bim::game::contest_result
bim::net::contest_runner::run(std::chrono::nanoseconds elapsed_wall_time)
{
  if (m_contest_result)
    return *m_contest_result;

  const int tick_count =
      m_tick_counter.add(elapsed_wall_time, bim::game::contest::tick_interval);
  bim_assume(tick_count >= 0);

  entt::registry& registry = m_contest.registry();

  const bim::game::player_action* const player_current_action_ptr =
      bim::game::find_player_action_by_index(registry, m_local_player_index);

  // Start by storing the queued actions because the restoration of the
  // state thereafter will overwrite them.
  const bim::game::player_action player_current_action_copy =
      player_current_action_ptr ? *player_current_action_ptr
                                : bim::game::player_action{};

  sync_with_server(registry);

  for (int i = 0; i != tick_count; ++i)
    {
      apply_actions_for_current_tick(registry, player_current_action_copy);
      m_update_exchange.push(player_current_action_copy);
      m_contest.tick();
    }

  m_last_completed_tick += tick_count;

  return bim::game::contest_result::create_still_running();
}

void bim::net::contest_runner::queue_updates(const server_update& updates)
{
  std::size_t tick_count = 0;

  for (int i = 0; i != m_player_count; ++i)
    {
      const std::vector<bim::game::player_action>& remote_actions =
          updates.actions[i];
      std::vector<bim::game::player_action>& local_actions =
          m_server_actions[i];

      if (local_actions.size() > tick_count)
        tick_count = local_actions.size();

      local_actions.insert(local_actions.end(), remote_actions.begin(),
                           remote_actions.end());
    }

  assert(updates.from_tick == m_last_confirmed_tick + tick_count);
}

void bim::net::contest_runner::sync_with_server(entt::registry& registry)
{
  restore_last_confirmed_state(registry);

  bool has_action = false;
  for (const std::vector<bim::game::player_action>& actions : m_server_actions)
    has_action |= !actions.empty();

  if (has_action)
    {
      apply_server_actions(registry);
      save_contest_state(registry);
      drop_confirmed_actions();
    }

  apply_unconfirmed_actions(registry);
}

void bim::net::contest_runner::restore_last_confirmed_state(
    entt::registry& registry)
{
  registry.clear();

  archive_io(entt::snapshot_loader(registry),
             bim::game::input_archive(m_last_confirmed_archive.data()));
  m_contest.arena(m_last_confirmed_arena);
}

void bim::net::contest_runner::apply_server_actions(entt::registry& registry)
{
  std::size_t tick_count = 0;

  for (int player_index = 0; player_index != m_player_count; ++player_index)
    if (m_server_actions[player_index].size() > tick_count)
      tick_count = m_server_actions[player_index].size();

  const std::array<std::size_t, bim::game::g_max_player_count> kick_tick =
      bim::game::find_kick_event_tick(m_server_actions, tick_count);

  for (std::size_t tick = 0; tick != tick_count; ++tick)
    {
      registry.view<bim::game::player, bim::game::player_action>().each(
          [this, tick](const bim::game::player& p,
                       bim::game::player_action& action) -> void
          {
            const std::vector<bim::game::player_action>& actions =
                m_server_actions[p.index];

            if (tick < actions.size())
              action = actions[tick];
          });

      for (int i = 0; i != m_player_count; ++i)
        if (tick == kick_tick[i])
          bim::game::kick_player(registry, i);

      m_contest.tick();
    }

  // Keep the last confirmed actions of the other player such that we can
  // re-apply the movement in the non-confirmed ticks.
  for (int player_index = 0; player_index != m_player_count; ++player_index)
    if (player_index != m_local_player_index)
      {
        bim::game::player_action& action =
            m_unconfirmed_actions[player_index].back();

        if (m_server_actions[player_index].empty())
          action = {};
        else
          {
            action = m_server_actions[player_index].back();
            action.drop_bomb = false;
          }
      }

  m_last_confirmed_tick += tick_count;
  assert(m_last_confirmed_tick <= m_last_completed_tick);

  for (int player_index = 0; player_index != m_player_count; ++player_index)
    m_server_actions[player_index].clear();
}

void bim::net::contest_runner::save_contest_state(entt::registry& registry)
{
  m_last_confirmed_archive.clear();

  archive_io(entt::snapshot(registry),
             bim::game::output_archive(m_last_confirmed_archive));
  m_last_confirmed_arena = m_contest.arena();
}

void bim::net::contest_runner::drop_confirmed_actions()
{
  bim_assume(m_last_completed_tick >= m_last_confirmed_tick);
  const std::size_t keep_count = m_last_completed_tick - m_last_confirmed_tick;
  std::vector<bim::game::player_action>& actions =
      m_unconfirmed_actions[m_local_player_index];

  bim_assume(keep_count <= actions.size());
  actions.erase(actions.begin(), actions.end() - keep_count);
}

void bim::net::contest_runner::apply_unconfirmed_actions(
    entt::registry& registry)
{
  for (std::size_t i = 0,
                   n = m_unconfirmed_actions[m_local_player_index].size();
       i != n; ++i)
    {
      for (int player_index = 0; player_index != m_player_count;
           ++player_index)
        {
          bim::game::player_action* const player_action =
              bim::game::find_player_action_by_index(m_contest.registry(),
                                                     player_index);

          if (player_action)
            {
              const std::vector<bim::game::player_action>& actions =
                  m_unconfirmed_actions[player_index];

              // Local players: apply the action they did in tick i.
              // Distant players: repeat the last action received from the
              // server.
              *player_action = actions[std::min(actions.size() - 1, i)];
            }
        }

      m_contest.tick();
    }
}

void bim::net::contest_runner::apply_actions_for_current_tick(
    entt::registry& registry,
    const bim::game::player_action& local_player_action)
{
  m_unconfirmed_actions[m_local_player_index].push_back(local_player_action);

  registry.view<bim::game::player, bim::game::player_action>().each(
      [this](const bim::game::player& p,
             bim::game::player_action& action) -> void
      {
        action = m_unconfirmed_actions[p.index].back();
      });
}

template <typename Archive, typename Snapshot>
void bim::net::contest_runner::archive_io(Snapshot&& snapshot,
                                          Archive&& archive) const
{
  snapshot.template get<entt::entity>(archive)
      .template get<bim::game::animation_state>(archive)
      .template get<bim::game::arena_reduction_state>(archive)
      .template get<bim::game::bomb>(archive)
      .template get<bim::game::bomb_power_up>(archive)
      .template get<bim::game::bomb_power_up_spawner>(archive)
      .template get<bim::game::brick_wall>(archive)
      .template get<bim::game::burning>(archive)
      .template get<bim::game::dead>(archive)
      .template get<bim::game::falling_block>(archive)
      .template get<bim::game::flame>(archive)
      .template get<bim::game::flame_power_up>(archive)
      .template get<bim::game::flame_power_up_spawner>(archive)
      .template get<bim::game::fog_of_war>(archive)
      .template get<bim::game::fractional_position_on_grid>(archive)
      .template get<bim::game::game_timer>(archive)
      .template get<bim::game::invincibility_state>(archive)
      .template get<bim::game::invisibility_power_up>(archive)
      .template get<bim::game::invisibility_power_up_spawner>(archive)
      .template get<bim::game::invisibility_state>(archive)
      .template get<bim::game::shield>(archive)
      .template get<bim::game::shield_power_up>(archive)
      .template get<bim::game::shield_power_up_spawner>(archive)
      .template get<bim::game::kicked>(archive)
      .template get<bim::game::player>(archive)
      .template get<bim::game::player_action>(archive)
      .template get<bim::game::player_action_queue>(archive)
      .template get<bim::game::position_on_grid>(archive)
      .template get<bim::game::timer>(archive);
}
