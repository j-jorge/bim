#include <bm/game/arena.hpp>

bm::game::arena::arena() = default;

bm::game::arena::arena(uint8_t width, uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_cells(width * height)
{}

uint8_t bm::game::arena::width() const
{
  return m_width;
}

uint8_t bm::game::arena::height() const
{
  return m_height;
}
