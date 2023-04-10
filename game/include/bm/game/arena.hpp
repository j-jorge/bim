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
      arena(uint8_t width, uint8_t height);

      uint8_t width() const;
      uint8_t height() const;

    private:
      uint8_t m_width;
      uint8_t m_height;

      std::vector<entity> m_cells;
    };
  }
}
