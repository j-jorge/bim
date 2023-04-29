#include <bm/game/arena.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/position_on_grid.hpp>

#include <entt/entity/entity.hpp>

bm::game::arena::arena(std::uint8_t width, std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_entities(width * height, (entt::entity)entt::null)
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

void bm::game::arena::put_entity(std::uint8_t x, std::uint8_t y,
                                 entt::entity e)
{
  m_entities[y * m_width + x] = e;
}

bool bm::game::arena::is_static_wall(std::uint8_t x, std::uint8_t y) const
{
  return m_walls[y * m_width + x];
}

void bm::game::arena::set_static_wall(std::uint8_t x, std::uint8_t y)
{
  m_walls[y * m_width + x] = true;
}
