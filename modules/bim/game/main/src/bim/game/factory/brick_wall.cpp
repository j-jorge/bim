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
#include <bim/game/factory/brick_wall.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::brick_wall_factory(entt::registry& registry,
                                           arena& arena, std::uint8_t x,
                                           std::uint8_t y)
{
  const entt::entity entity = registry.create();

  registry.emplace<brick_wall>(entity);
  registry.emplace<position_on_grid>(entity, x, y);

  arena.put_entity(x, y, entity);

  return entity;
}
