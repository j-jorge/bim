// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/component/position_on_grid.hpp>

#include <bim/assume.hpp>

#include <entt/entity/entity.hpp>

bim::game::arena::arena() = default;

bim::game::arena::arena(std::uint8_t width, std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_entities(width * height, (entt::entity)entt::null)
  , m_walls(width * height, false)
  , m_solids(width * height, false)
{}

std::uint8_t bim::game::arena::width() const
{
  return m_width;
}

std::uint8_t bim::game::arena::height() const
{
  return m_height;
}

entt::entity bim::game::arena::entity_at(std::uint8_t x, std::uint8_t y) const
{
  return m_entities[y * m_width + x];
}

void bim::game::arena::put_entity(std::uint8_t x, std::uint8_t y,
                                  entt::entity e)
{
  assert(entity_at(x, y) == entt::null);
  m_entities[y * m_width + x] = e;
}

void bim::game::arena::erase_entity(std::uint8_t x, std::uint8_t y)
{
  m_entities[y * m_width + x] = entt::null;
  m_solids[y * m_width + x] = false;
}

bool bim::game::arena::is_solid(std::uint8_t x, std::uint8_t y) const
{
  return m_solids[y * m_width + x];
}

void bim::game::arena::set_solid(std::uint8_t x, std::uint8_t y)
{
  m_solids[y * m_width + x] = true;
}

bool bim::game::arena::is_static_wall(std::uint8_t x, std::uint8_t y) const
{
  return m_walls[y * m_width + x];
}

void bim::game::arena::set_static_wall(std::uint8_t x, std::uint8_t y)
{
  m_walls[y * m_width + x] = true;
  m_solids[y * m_width + x] = true;
}
