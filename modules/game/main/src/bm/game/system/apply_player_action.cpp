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
#include <bm/game/system/apply_player_action.hpp>

#include <bm/game/arena.hpp>

#include <bm/game/component/player.hpp>
#include <bm/game/component/player_action.hpp>
#include <bm/game/component/player_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>
#include <bm/game/factory/bomb.hpp>

#include <entt/entity/registry.hpp>

static void drop_bomb(entt::registry& registry, bm::game::arena& arena,
                      const bm::game::player& player,
                      const bm::game::position_on_grid& position)
{
  if (arena.entity_at(position.x, position.y) != entt::null)
    return;

  arena.put_entity(position.x, position.y,
                   bm::game::bomb_factory(registry, position.x, position.y,
                                          player.bomb_strength));
}

static void move_player(bm::game::player& player,
                        bm::game::position_on_grid& position,
                        bm::game::player_direction direction,
                        const bm::game::arena& arena)
{
  player.current_direction = direction;

  int x = position.x;
  int y = position.y;

  switch (direction)
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
      && !arena.is_static_wall(x, y) && (arena.entity_at(x, y) == entt::null))
    {
      position.x = x;
      position.y = y;
    }
}

void bm::game::apply_player_action(entt::registry& registry, arena& arena)
{
  registry.view<player, position_on_grid, player_action>().each(
      [&registry, &arena](player& player, position_on_grid& position,
                          player_action& action) -> void
      {
        if (action.drop_bomb)
          drop_bomb(registry, arena, player, position);

        if (action.requested_direction)
          move_player(player, position, *action.requested_direction, arena);

        action = player_action{};
      });
}