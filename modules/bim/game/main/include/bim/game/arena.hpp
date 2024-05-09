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
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <vector>

namespace bim::game
{
  class arena
  {
  public:
    arena();
    arena(std::uint8_t width, std::uint8_t height);

    std::uint8_t width() const;
    std::uint8_t height() const;

    entt::entity entity_at(std::uint8_t x, std::uint8_t y) const;
    void put_entity(std::uint8_t x, std::uint8_t y, entt::entity e);
    void erase_entity(std::uint8_t x, std::uint8_t y);

    bool is_solid(std::uint8_t x, std::uint8_t y) const;
    void set_solid(std::uint8_t x, std::uint8_t y);

    bool is_static_wall(std::uint8_t x, std::uint8_t y) const;
    void set_static_wall(std::uint8_t x, std::uint8_t y);

  private:
    std::uint8_t m_width;
    std::uint8_t m_height;

    std::vector<entt::entity> m_entities;
    std::vector<bool> m_walls;
    std::vector<bool> m_solids;
  };
}
