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
#include <bm/game/factory/flame.hpp>

#include <bm/game/component/flame.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bm::game::flame_factory(entt::registry& registry, std::uint8_t x,
                                     std::uint8_t y,
                                     flame_horizontal horizontal,
                                     flame_vertical vertical, flame_end end)
{
  return flame_factory(registry, x, y, horizontal, vertical, end,
                       std::chrono::milliseconds(800));
}

entt::entity bm::game::flame_factory(entt::registry& registry, std::uint8_t x,
                                     std::uint8_t y,
                                     flame_horizontal horizontal,
                                     flame_vertical vertical, flame_end end,
                                     std::chrono::milliseconds time_to_live)
{
  const entt::entity entity = registry.create();

  registry.emplace<flame>(entity, horizontal, vertical, end, time_to_live);
  registry.emplace<position_on_grid>(entity, x, y);

  return entity;
}
