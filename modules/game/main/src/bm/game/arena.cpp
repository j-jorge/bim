/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bm/game/arena.hpp>

#include <bm/game/component/position_on_grid.hpp>

#include <bm/game/assume.hpp>

#include <entt/entity/entity.hpp>

bm::game::arena::arena() = default;

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
  assert(entity_at(x, y) == entt::null);
  m_entities[y * m_width + x] = e;
}

void bm::game::arena::erase_entity(std::uint8_t x, std::uint8_t y)
{
  m_entities[y * m_width + x] = entt::null;
}

bool bm::game::arena::is_static_wall(std::uint8_t x, std::uint8_t y) const
{
  return m_walls[y * m_width + x];
}

void bm::game::arena::set_static_wall(std::uint8_t x, std::uint8_t y)
{
  m_walls[y * m_width + x] = true;
}
