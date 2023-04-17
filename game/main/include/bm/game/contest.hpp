#pragma once

#include <bm/game/arena.hpp>
#include <bm/game/bomb.hpp>
#include <bm/game/flame.hpp>
#include <bm/game/player.hpp>

#include <span>

namespace bm
{
  namespace game
  {
    class contest
    {
    public:
      contest(entt::registry& registry, int player_count, int arena_width,
              int arena_height);

      void tick();

      const bm::game::arena& arena() const;
      std::span<const player> players() const;
      std::span<const flame> flames() const;
      std::span<const bomb> bombs() const;

    private:
      bm::game::arena m_arena;
      std::vector<player> m_players;
      std::vector<flame> m_flames;
      std::vector<bomb> m_bombs;
    };
  }
}
