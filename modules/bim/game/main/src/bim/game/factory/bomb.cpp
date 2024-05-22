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
#include <bim/game/factory/bomb.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::bomb_factory(entt::registry& registry, std::uint8_t x,
                                     std::uint8_t y, std::uint8_t strength,
                                     std::uint8_t player_index)
{
  return bomb_factory(registry, x, y, strength, player_index,
                      std::chrono::seconds(3));
}

entt::entity
bim::game::bomb_factory(entt::registry& registry, std::uint8_t x,
                        std::uint8_t y, std::uint8_t strength,
                        std::uint8_t player_index,
                        std::chrono::milliseconds duration_until_explosion)
{
  const entt::entity entity = registry.create();

  registry.emplace<bomb>(entity, duration_until_explosion, strength,
                         player_index);
  registry.emplace<position_on_grid>(entity, x, y);

  return entity;
}
