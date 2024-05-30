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
#include <bim/net/contest_runner.hpp>

#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/exchange/server_update.hpp>

#include <bim/game/check_game_over.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/input_archive.hpp>
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
  const int tick_count =
      m_tick_counter.add(elapsed_wall_time, bim::game::contest::tick_interval);
  bim_assume(tick_count >= 0);

  if (tick_count == 0)
    return bim::game::contest_result::create_still_running();

  entt::registry& registry = m_contest.registry();

  const bim::game::player_action* const player_current_action_ptr =
      bim::game::find_player_action_by_index(registry, m_local_player_index);

  // Start by storing the queued actions because the restoration of the
  // state thereafter will overwrite them.
  const bim::game::player_action player_current_action_copy =
      player_current_action_ptr ? *player_current_action_ptr
                                : bim::game::player_action{};

  if (!m_server_actions[0].empty())
    {
      sync_with_server(registry);

      const bim::game::contest_result result =
          bim::game::check_game_over(registry);

      if (!result.still_running())
        return result;

      apply_unconfirmed_actions(registry);
    }

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
  assert(updates.from_tick
         == m_last_confirmed_tick + m_server_actions[0].size());

  for (int i = 0; i != m_player_count; ++i)
    m_server_actions[i].insert(m_server_actions[i].end(),
                               updates.actions[i].begin(),
                               updates.actions[i].end());
}

void bim::net::contest_runner::sync_with_server(entt::registry& registry)
{
  restore_last_confirmed_state(registry);
  apply_server_actions(registry);
  save_contest_state(registry);
  drop_confirmed_actions();
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
  const std::size_t tick_count = m_server_actions[0].size();

  for (std::size_t tick = 0; tick != tick_count; ++tick)
    {
      registry.view<bim::game::player, bim::game::player_action>().each(
          [this, tick](const bim::game::player& p,
                       bim::game::player_action& action) -> void
          {
            action = m_server_actions[p.index][tick];
          });

      m_contest.tick();
    }

  // Keep the last confirmed actions of the other player such that we can
  // re-apply the movement in the non-confirmed ticks.
  for (int player_index = 0; player_index != m_player_count; ++player_index)
    if (player_index != m_local_player_index)
      {
        bim::game::player_action& action =
            m_unconfirmed_actions[player_index].back();
        action = m_server_actions[player_index].back();
        action.drop_bomb = false;
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
  std::array<bim::game::player_action*, 4> player_action_pointers{};

  registry.view<bim::game::player, bim::game::player_action>().each(
      [&player_action_pointers](const bim::game::player& p,
                                bim::game::player_action& action) -> void
      {
        player_action_pointers[p.index] = &action;
      });

  for (std::size_t i = 0,
                   n = m_unconfirmed_actions[m_local_player_index].size();
       i != n; ++i)
    {
      for (int player_index = 0; player_index != m_player_count;
           ++player_index)
        if (player_action_pointers[player_index])
          {
            const std::vector<bim::game::player_action>& actions =
                m_unconfirmed_actions[player_index];

            // Local players: apply the action they did in tick i.
            // Distant players: repeat the last action received from the
            // server.
            *player_action_pointers[player_index] =
                actions[std::min(actions.size() - 1, i)];
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
  snapshot.entities(archive)
      .template component<
          bim::game::bomb, bim::game::bomb_power_up,
          bim::game::bomb_power_up_spawner, bim::game::brick_wall,
          bim::game::burning, bim::game::dead, bim::game::flame,
          bim::game::flame_power_up, bim::game::flame_power_up_spawner,
          bim::game::fractional_position_on_grid, bim::game::player_action,
          bim::game::player_action_queue, bim::game::player,
          bim::game::position_on_grid>(archive);
}
