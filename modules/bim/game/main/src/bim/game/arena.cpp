// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/static_wall.hpp>

#include <bim/assume.hpp>
#include <bim/table_2d.impl.hpp>

#include <entt/entity/entity.hpp>

bim::game::arena::arena() = default;

bim::game::arena::arena(std::uint8_t width, std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_entities(width, height, (entt::entity)entt::null)
  , m_is_static_wall(width, height, false)
  , m_solids(width, height, false)
{
  const int border = 2 * (width + height - 2);
  const int inside = ((width - 2) / 2) * ((height - 2) / 2);

  m_static_walls.reserve(std::max(border + inside, 0));
}

bim::game::arena::arena(const arena& that) noexcept = default;
bim::game::arena::arena(arena&& that) noexcept = default;
bim::game::arena::~arena() = default;
bim::game::arena&
bim::game::arena::operator=(const arena& that) noexcept = default;
bim::game::arena& bim::game::arena::operator=(arena&& that) noexcept = default;

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
  return m_entities(x, y);
}

void bim::game::arena::put_entity(std::uint8_t x, std::uint8_t y,
                                  entt::entity e)
{
  assert(entity_at(x, y) == entt::null);
  m_entities(x, y) = e;
}

void bim::game::arena::erase_entity(std::uint8_t x, std::uint8_t y)
{
  m_entities(x, y) = entt::null;
  m_solids(x, y) = false;
}

bool bim::game::arena::is_blocker(std::uint8_t x, std::uint8_t y) const
{
  return m_solids(x, y) || m_is_static_wall(x, y);
}

bool bim::game::arena::is_solid(std::uint8_t x, std::uint8_t y) const
{
  return m_solids(x, y);
}

void bim::game::arena::set_solid(std::uint8_t x, std::uint8_t y)
{
  m_solids(x, y) = true;
}

std::span<const bim::game::static_wall> bim::game::arena::static_walls() const
{
  return { m_static_walls.begin(), m_static_walls.end() };
}

bool bim::game::arena::is_static_wall(std::uint8_t x, std::uint8_t y) const
{
  return m_is_static_wall(x, y);
}

void bim::game::arena::set_static_wall(std::uint8_t x, std::uint8_t y,
                                       cell_neighborhood n)
{
  m_static_walls.emplace_back(static_wall{ x, y, n });
  m_is_static_wall(x, y) = true;
}
