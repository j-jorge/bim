// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

#include <chrono>
#include <memory>

namespace bim::game
{
  class arena;
  class arena_reduction;
  class contest_result;
  class context;
  class entity_world_map;
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

    const bim::game::context& context() const;

    entt::registry& registry();
    const entt::registry& registry() const;

    const bim::game::arena& arena() const;

    const bim::game::entity_world_map& entity_map() const;
    void entity_map(const bim::game::entity_world_map& m);

    const bim::table_2d<bim::game::fog_of_war*>&
    fog_map(std::size_t player_index) const;

  private:
    const std::unique_ptr<entt::registry> m_registry;
    const std::unique_ptr<bim::game::context> m_context;
    const std::unique_ptr<bim::game::arena> m_arena;
    const std::unique_ptr<entity_world_map> m_entity_world_map;

    std::unique_ptr<arena_reduction> m_arena_reduction;
    std::unique_ptr<fog_of_war_updater> m_fog_of_war;
  };
}
