#pragma once

#include <bm/game/arena.hpp>
#include <bm/game/random_generator.hpp>

#include <entt/entity/registry.hpp>

namespace bm
{
  namespace game
  {
    class contest
    {
    public:
      contest(std::uint64_t seed, std::uint8_t brick_wall_probability,
              std::uint8_t player_count, std::uint8_t arena_width,
              std::uint8_t arena_height);

      void tick();

      const entt::registry& registry() const;
      const bm::game::arena& arena() const;

    private:
      entt::registry m_registry;
      bm::game::random_generator m_random;
      bm::game::arena m_arena;
    };
  }
}
