#include <bm/game/arena.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/position_on_grid.hpp>
#include <bm/game/wall.hpp>

#include <entt/entity/registry.hpp>

bm::game::arena::arena(entt::registry& registry, std::uint8_t width,
                       std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_cells(width * height)
{
  bm_assume(width >= 3);
  bm_assume(height >= 3);

  for(entt::entity& e : m_cells)
    e = registry.create();

  // Positions on the grid, such that we can get them for the entities.
  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      registry.emplace<position_on_grid>(at(x, y), x, y);

  // Unbreakable walls on the borders.
  for(int x = 0; x != width; ++x)
    registry.emplace<wall>(at(x, 0));

  for(int x = 0; x != width; ++x)
    registry.emplace<wall>(at(x, height - 1));

  for(int y = 1; y < height - 1; ++y)
    registry.emplace<wall>(at(0, y));

  for(int y = 1; y < height - 1; ++y)
    registry.emplace<wall>(at(width - 1, y));

  // Unbreakable walls in the game area.
  for(int y = 2; y < height - 2; y += 2)
    for(int x = 2; x < width - 2; x += 2)
      registry.emplace<wall>(m_cells[y * width + x]);
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
