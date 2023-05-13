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
#include <bm/game/system/update_flames.hpp>

#include <bm/game/arena.hpp>

#include <bm/game/component/flame.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

void bm::game::update_flames(entt::registry& registry, arena& arena,
                             std::chrono::milliseconds elapsed_time)
{
  registry.view<flame, position_on_grid>().each(
      [&](entt::entity e, flame& f, position_on_grid position) -> void
      {
        if (elapsed_time >= f.time_to_live)
          {
            registry.destroy(e);
            arena.erase_entity(position.x, position.y);
          }
        else
          f.time_to_live -= elapsed_time;
      });
}
