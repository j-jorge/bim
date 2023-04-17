#include <bm/game/arena.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

bm::game::arena::arena(entt::registry& registry, std::uint8_t width,
                       std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_cells(width * height)
{
  for(entt::entity& e : m_cells)
    e = registry.create();

  // Positions on the grid, such that we can get them for the entities.
  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      registry.emplace<position_on_grid>(at(x, y), x, y);
}

std::uint8_t bm::game::arena::width() const
{
  return m_width;
}

std::uint8_t bm::game::arena::height() const
{
  return m_height;
}

entt::entity bm::game::arena::at(uint8_t x, uint8_t y) const
{
  return m_cells[y * m_width + x];
}
