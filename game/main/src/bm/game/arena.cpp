#include <bm/game/arena.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/position_on_grid.hpp>

bm::game::arena::arena(std::uint8_t width, std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_entities(width * height, entt::entity{})
  , m_walls(width * height, false)
{}

std::uint8_t bm::game::arena::width() const
{
  return m_width;
}

std::uint8_t bm::game::arena::height() const
{
  return m_height;
}

entt::entity bm::game::arena::entity_at(std::uint8_t x, std::uint8_t y) const
{
  return m_entities[y * m_width + x];
}

bool bm::game::arena::is_static_wall(std::uint8_t x, std::uint8_t y) const
{
  return m_walls[y * m_width + x];
}

void bm::game::arena::set_static_wall(std::uint8_t x, std::uint8_t y)
{
  m_walls[y * m_width + x] = true;
}
