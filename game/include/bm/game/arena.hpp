#pragma once

#include <bm/game/arena.hpp>

namespace bm
{
  namespace game
  {
    class arena
    {
    public:
      arena();
      arena(int width, int height);

    private:
      std::vector<entity> m_cells;
    };
  }
}
