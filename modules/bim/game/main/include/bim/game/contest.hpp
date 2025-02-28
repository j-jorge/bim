// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/arena.hpp>
#include <bim/game/feature_flags_fwd.hpp>

#include <bim/table_2d.hpp>

#include <entt/entity/registry.hpp>

#include <chrono>
#include <memory>

namespace bim::game
{
  class contest_result;
  class arena_reduction;
  class fog_of_war_updater;
  struct contest_fingerprint;
  struct fog_of_war;

  class contest
  {
  public:
    static constexpr std::chrono::milliseconds tick_interval =
        std::chrono::milliseconds(20);

  public:
    explicit contest(const contest_fingerprint& fingerprint);
    contest(const contest_fingerprint& fingerprint,
            std::uint8_t local_player_index);
    ~contest();

    contest_result tick();

    entt::registry& registry();
    const entt::registry& registry() const;
    const bim::game::arena& arena() const;
    void arena(const bim::game::arena& a);

    const bim::table_2d<bim::game::fog_of_war*>&
    fog_map(std::size_t player_index) const;

  private:
    entt::registry m_registry;
    bim::game::arena m_arena;

    std::unique_ptr<arena_reduction> m_arena_reduction;
    std::unique_ptr<fog_of_war_updater> m_fog_of_war;
  };
}
