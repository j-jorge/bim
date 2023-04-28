#pragma once

#include <bm/game/arena.hpp>

namespace bm
{
  namespace game
  {
    class contest
    {
    public:
      contest(entt::registry& registry, std::uint8_t player_count,
              std::uint8_t arena_width, std::uint8_t arena_height);

      void tick();

      const bm::game::arena& arena() const;

    private:
      bm::game::arena m_arena;
    };
  }
}
