#include <bm/game/arena.hpp>

#include <entt/entity/registry.hpp>

bm::game::arena::arena(entt::registry& registry, std::uint8_t width,
                       std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_cells(width * height)
{
  for(entt::entity& e : m_cells)
    e = registry.create();
}

std::uint8_t bm::game::arena::width() const
{
  return m_width;
}

std::uint8_t bm::game::arena::height() const
{
  return m_height;
}
