#include <bm/game/arena.hpp>

bm::game::arena::arena() = default;

bm::game::arena::arena(int width, int height)
  : m_cells(width * height)
{}
