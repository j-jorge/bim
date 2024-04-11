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
#include <bim/game/factory/flame.hpp>

#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::flame_factory(entt::registry& registry, std::uint8_t x,
                                      std::uint8_t y,
                                      flame_direction direction,
                                      flame_segment segment)
{
  return flame_factory(registry, x, y, direction, segment,
                       std::chrono::milliseconds(800));
}

entt::entity bim::game::flame_factory(entt::registry& registry, std::uint8_t x,
                                      std::uint8_t y,
                                      flame_direction direction,
                                      flame_segment segment,
                                      std::chrono::milliseconds time_to_live)
{
  const entt::entity entity = registry.create();

  registry.emplace<flame>(entity, direction, segment, time_to_live);
  registry.emplace<position_on_grid>(entity, x, y);

  return entity;
}
