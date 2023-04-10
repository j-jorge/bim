#pragma once

#include <bm/game/entity.hpp>

#include <vector>

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
