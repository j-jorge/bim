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
#include <bm/net/contest_runner.hpp>

#include <bm/net/exchange/game_update_exchange.hpp>
#include <bm/net/exchange/server_update.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/component/bomb.hpp>
#include <bm/game/component/brick_wall.hpp>
#include <bm/game/component/burning.hpp>
#include <bm/game/component/flame.hpp>
#include <bm/game/component/player.hpp>
#include <bm/game/component/player_action.hpp>
#include <bm/game/component/position_on_grid.hpp>
#include <bm/game/contest.hpp>
#include <bm/game/input_archive.hpp>
#include <bm/game/output_archive.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

bm::net::contest_runner::contest_runner(bm::game::contest& contest,
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
  m_server_actions.reserve(64);

  save_contest_state(contest.registry());

  for (int player_index = 0; player_index != player_count; ++player_index)
    if (player_index != local_player_index)
      m_unconfirmed_actions[player_index].emplace_back(
          bm::game::player_action{});

  update_exchange.connect_to_updated(
      std::bind(&contest_runner::queue_updates, this, std::placeholders::_1));
}

void bm::net::contest_runner::run(std::chrono::nanoseconds elapsed_wall_time)
{
  const int tick_count = m_tick_counter.add(elapsed_wall_time,
                                            bm::game::contest::tick_interval);
  bm_assume(tick_count >= 0);

  if (tick_count == 0)
    return;

  entt::registry& registry = m_contest.registry();

  // Start by storing the queued actions because the restoration of the
  // state thereafter will overwrite them.
  const bm::game::player_action player_current_action_copy
      = find_local_player_action(registry);

  if (!m_server_actions.empty())
    {
      sync_with_server(registry);
      apply_unconfirmed_actions(registry);
    }

  apply_actions_for_current_tick(registry, player_current_action_copy);

  for (int i = 0; i != tick_count; ++i)
    {
      m_update_exchange.push(find_local_player_action(registry));
      m_contest.tick();
    }

  m_last_completed_tick += tick_count;
}

void bm::net::contest_runner::queue_updates(const server_update& updates)
{
  assert(updates.from_tick == m_last_confirmed_tick + m_server_actions.size());

  m_server_actions.insert(m_server_actions.end(), updates.actions.begin(),
                          updates.actions.end());
}

bm::game::player_action& bm::net::contest_runner::find_local_player_action(
    entt::registry& registry) const
{
  for (auto&& [entity, player, action] :
       registry.view<bm::game::player, bm::game::player_action>().each())
    if (player.index == m_local_player_index)
      return action;

  bm_assume(false);
}

void bm::net::contest_runner::sync_with_server(entt::registry& registry)
{
  restore_last_confirmed_state(registry);
  apply_server_actions(registry);
  save_contest_state(registry);
  drop_confirmed_actions();
}

void bm::net::contest_runner::restore_last_confirmed_state(
    entt::registry& registry)
{
  registry.clear();

  archive_io(entt::snapshot_loader(registry),
             bm::game::input_archive(m_last_confirmed_archive.data()));
  m_contest.arena(m_last_confirmed_arena);
}

void bm::net::contest_runner::apply_server_actions(entt::registry& registry)
{
  for (const std::array<bm::game::player_action, 4>& server_action :
       m_server_actions)
    {
      registry.view<bm::game::player, bm::game::player_action>().each(
          [&server_action](const bm::game::player& p,
                           bm::game::player_action& action) -> void
          {
            action = server_action[p.index];
          });

      m_contest.tick();
    }

  // Keep the last confirmed actions of the other player such that we can
  // re-apply it in the non-confirmed ticks.
  for (int player_index = 0; player_index != m_player_count; ++player_index)
    if (player_index != m_local_player_index)
      // TODO: remove the drop bomb actions.
      m_unconfirmed_actions[player_index].back()
          = m_server_actions.back()[player_index];

  m_last_confirmed_tick += m_server_actions.size();
  assert(m_last_confirmed_tick <= m_last_completed_tick);

  m_server_actions.clear();
}

void bm::net::contest_runner::save_contest_state(entt::registry& registry)
{
  m_last_confirmed_archive.clear();

  archive_io(entt::snapshot(registry),
             bm::game::output_archive(m_last_confirmed_archive));
  m_last_confirmed_arena = m_contest.arena();
}

void bm::net::contest_runner::drop_confirmed_actions()
{
  bm_assume(m_last_completed_tick >= m_last_confirmed_tick);
  const std::size_t keep_count = m_last_completed_tick - m_last_confirmed_tick;
  std::vector<bm::game::player_action>& actions
      = m_unconfirmed_actions[m_local_player_index];

  bm_assume(keep_count <= actions.size());
  actions.erase(actions.begin(), actions.end() - keep_count);
}

void bm::net::contest_runner::apply_unconfirmed_actions(
    entt::registry& registry)
{
  std::array<bm::game::player_action*, 4> player_action_pointers;

  registry.view<bm::game::player, bm::game::player_action>().each(
      [&player_action_pointers](const bm::game::player& p,
                                bm::game::player_action& action) -> void
      {
        player_action_pointers[p.index] = &action;
      });

  for (std::size_t i = 0,
                   n = m_unconfirmed_actions[m_local_player_index].size();
       i != n; ++i)
    {
      for (int player_index = 0; player_index != m_player_count;
           ++player_index)
        {
          const std::vector<bm::game::player_action>& actions
              = m_unconfirmed_actions[player_index];

          // Local players: apply the action they did in tick i.
          // Distant players: repeat the last action received from the
          // server.
          *player_action_pointers[player_index]
              = actions[std::min(actions.size() - 1, i)];
        }

      m_contest.tick();
    }
}

void bm::net::contest_runner::apply_actions_for_current_tick(
    entt::registry& registry,
    const bm::game::player_action& local_player_action)
{
  m_unconfirmed_actions[m_local_player_index].push_back(local_player_action);

  registry.view<bm::game::player, bm::game::player_action>().each(
      [this](const bm::game::player& p,
             bm::game::player_action& action) -> void
      {
        action = m_unconfirmed_actions[p.index].back();
      });
}

template <typename Archive, typename Snapshot>
void bm::net::contest_runner::archive_io(Snapshot&& snapshot,
                                         Archive&& archive) const
{
  snapshot.entities(archive)
      .template component<bm::game::bomb, bm::game::brick_wall,
                          bm::game::burning, bm::game::flame,
                          bm::game::player_action, bm::game::player,
                          bm::game::position_on_grid>(archive);
}
