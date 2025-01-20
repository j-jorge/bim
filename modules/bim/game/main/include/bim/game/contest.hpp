// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/arena.hpp>
#include <bim/game/feature_flags_fwd.hpp>

#include <entt/entity/registry.hpp>

#include <chrono>
#include <memory>

namespace bim::game
{
  class contest_result;
  class arena_reduction;

  class contest
  {
  public:
    static constexpr std::chrono::milliseconds tick_interval =
        std::chrono::milliseconds(20);

  public:
    contest(std::uint64_t seed, std::uint8_t brick_wall_probability,
            std::uint8_t player_count, std::uint8_t arena_width,
            std::uint8_t arena_height, feature_flags features);
    ~contest();

    contest_result tick();

    entt::registry& registry();
    const entt::registry& registry() const;
    const bim::game::arena& arena() const;
    void arena(const bim::game::arena& a);

  private:
    entt::registry m_registry;
    bim::game::arena m_arena;

    std::unique_ptr<arena_reduction> m_arena_reduction;
  };
}
