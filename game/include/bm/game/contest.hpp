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
      contest();
      contest(int player_count, int arena_width, int arena_height);

      void tick();

      const arena& arena() const;
      std::span<const player> players() const;
      std::span<const flame> flames() const;
      std::span<const bomb> bombs() const;

    private:
      arena m_arena;
      std::vector<player> m_players;
      std::vector<flame> m_flames;
      std::vector<bomb> m_bombs;
    };
  }
}
