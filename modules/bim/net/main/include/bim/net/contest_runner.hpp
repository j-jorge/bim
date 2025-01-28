// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/archive_storage.hpp>
#include <bim/game/arena.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/tick_counter.hpp>

#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <optional>

namespace bim::game
{
  class contest;
  struct player_action;
}

namespace bim::net
{
  class game_update_exchange;
  struct server_update;

  class contest_runner
  {
  public:
    contest_runner(bim::game::contest& contest,
                   game_update_exchange& update_exchange,
                   std::uint8_t local_player_index, std::uint8_t player_count);

    std::uint32_t local_tick() const;
    std::uint32_t confirmed_tick() const;

    bim::game::contest_result run(std::chrono::nanoseconds elapsed_wall_time);

  private:
    struct history_entry;

  private:
    void queue_updates(const server_update& updates);

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

    std::array<std::vector<bim::game::player_action>,
               bim::game::g_max_player_count>
        m_server_actions;
    std::uint32_t m_last_confirmed_tick;
    std::uint32_t m_last_completed_tick;

    std::array<std::vector<bim::game::player_action>,
               bim::game::g_max_player_count>
        m_unconfirmed_actions;

    bim::game::archive_storage m_last_confirmed_archive;
    bim::game::arena m_last_confirmed_arena;

    std::optional<bim::game::contest_result> m_contest_result;
  };
}
