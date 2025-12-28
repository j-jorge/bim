// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/cell_edge.hpp>
#include <bim/game/static_wall.hpp>

#include <bim/assume.hpp>
#include <bim/table_2d.impl.hpp>

bim::game::arena::arena() = default;

bim::game::arena::arena(std::uint8_t width, std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_is_static_wall(width, height, false)
  , m_fences(width, height, cell_edge::none)
{
  const int border = 2 * (width + height - 2);
  const int inside = ((width - 2) / 2) * ((height - 2) / 2);
  const int cell_count = std::max(border + inside, 0);

  m_static_walls.reserve(cell_count);
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
  assert(m_fences(x, y) == cell_edge::none);
  assert(!m_is_static_wall(x, y));

  m_static_walls.emplace_back(static_wall{ x, y, n });
  m_is_static_wall(x, y) = true;
}

bim::game::cell_edge bim::game::arena::fences(std::uint8_t x,
                                              std::uint8_t y) const
{
  return m_fences(x, y);
}

void bim::game::arena::add_fence(std::uint8_t x, std::uint8_t y, cell_edge e)
{
  assert(!m_is_static_wall(x, y));
  m_fences(x, y) |= e;
}

void bim::game::arena::remove_fence(std::uint8_t x, std::uint8_t y,
                                    cell_edge e)
{
  assert(!m_is_static_wall(x, y));
  m_fences(x, y) &= ~e;
}
