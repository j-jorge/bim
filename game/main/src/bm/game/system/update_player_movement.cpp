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

#include <bm/game/arena.hpp>

#include <bm/game/component/player.hpp>
#include <bm/game/component/player_action.hpp>
#include <bm/game/component/player_direction.hpp>

#include <entt/entity/registry.hpp>

static void update_player_movement(bm::game::player& player,
                                   const bm::game::player_action& action,
                                   const bm::game::arena& arena)
{
  int x = player.x;
  int y = player.y;

  if (action.requested)
    {
      switch (*action.requested)
        {
        case bm::game::player_direction::left:
          x -= 1;
          break;
        case bm::game::player_direction::right:
          x += 1;
          break;
        case bm::game::player_direction::up:
          y -= 1;
          break;
        case bm::game::player_direction::down:
          y += 1;
          break;
        }

      if ((x >= 0) && (y >= 0) && (x < arena.width()) && (y < arena.height())
          && !arena.is_static_wall(x, y)
          && (arena.entity_at(x, y) == entt::null))
        {
          player.x = x;
          player.y = y;
        }
    }
}

void bm::game::update_player_movement(entt::registry& registry,
                                      const arena& arena)
{
  registry.view<player, player_action>().each(
      [&arena](player& p, const player_action& a) -> void
      {
        ::update_player_movement(p, a, arena);
      });
}
