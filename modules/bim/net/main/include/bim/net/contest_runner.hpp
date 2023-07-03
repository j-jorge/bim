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
#pragma once

#include <bim/game/archive_storage.hpp>
#include <bim/game/arena.hpp>
#include <bim/game/tick_counter.hpp>

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class contest;
  class player_action;
}

namespace bim::net
{
  class game_update_exchange;
  class server_update;

  class contest_runner
  {
  public:
    contest_runner(bim::game::contest& contest,
                   game_update_exchange& update_exchange,
                   std::uint8_t local_player_index, std::uint8_t player_count);

    void run(std::chrono::nanoseconds elapsed_wall_time);

  private:
    struct history_entry;

  private:
    void queue_updates(const server_update& updates);

    bim::game::player_action&
    find_local_player_action(entt::registry& registry) const;

    void sync_with_server(entt::registry& registry);
    void restore_last_confirmed_state(entt::registry& registry);
    void apply_server_actions(entt::registry& registry);
    void save_contest_state(entt::registry& registry);
    void drop_confirmed_actions();
    void apply_unconfirmed_actions(entt::registry& registry);
    void apply_actions_for_current_tick(
        entt::registry& registry,
        const bim::game::player_action& local_player_action);

    template <typename Archive, typename Snapshot>
    void archive_io(Snapshot&& snapshot, Archive&& archive) const;

  private:
    const std::uint8_t m_local_player_index;
    const std::uint8_t m_player_count;

    bim::game::contest& m_contest;
    game_update_exchange& m_update_exchange;

    bim::game::tick_counter m_tick_counter;

    std::vector<std::array<bim::game::player_action, 4>> m_server_actions;
    std::uint32_t m_last_confirmed_tick;
    std::uint32_t m_last_completed_tick;

    std::array<std::vector<bim::game::player_action>, 4> m_unconfirmed_actions;

    bim::game::archive_storage m_last_confirmed_archive;
    bim::game::arena m_last_confirmed_arena;
  };
}
