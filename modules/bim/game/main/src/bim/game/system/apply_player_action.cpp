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
#include <bim/game/system/apply_player_action.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_kind.hpp>
#include <bim/game/component/player_direction.hpp>
#include <bim/game/factory/bomb.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

#include <fpm/math.hpp>

static void drop_bomb(entt::registry& registry, bim::game::arena& arena,
                      const bim::game::player& player,
                      const bim::game::fractional_position_on_grid& position)
{
  const std::uint8_t arena_x = position.grid_aligned_x();
  const std::uint8_t arena_y = position.grid_aligned_y();

  if (arena.entity_at(arena_x, arena_y) != entt::null)
    return;

  arena.put_entity(arena_x, arena_y,
                   bim::game::bomb_factory(registry, arena_x, arena_y,
                                           player.bomb_strength));
}

static void move_player(bim::game::player& player,
                        bim::game::fractional_position_on_grid& position,
                        bim::game::player_action_kind direction,
                        const bim::game::arena& arena)
{
  bim_assume(direction != bim::game::player_action_kind::drop_bomb);

  using position_t = bim::game::fractional_position_on_grid::value_type;

  constexpr position_t offset =
      position_t(1) / bim::game::g_player_steps_per_cell;
  constexpr position_t half = position_t(1) / 2;

  const position_t x = position.x;
  const position_t y = position.y;

  const position_t x_floor = fpm::floor(x);
  const position_t y_floor = fpm::floor(y);

  // Fractional position in the player's cell tells us if the player is on the
  // left or the right part of the cell. Same for the up/down parts.
  const position_t x_decimal = position.x - x_floor;
  const position_t y_decimal = position.y - y_floor;

  // Same as x_floor and y_floor but with a different type, for when we need
  // integers (e.g. cell coordinates in the arena).
  const std::uint8_t x_int = (std::uint8_t)x;
  const std::uint8_t y_int = (std::uint8_t)y;

  // If the player has moved, these variables are set to tell if we must check
  // for an obstacle at the given x or y. Only one is set since the player
  // moves in only one direction. E.g. move to the right -> check obstacles in
  // x_int + 1.
  std::uint8_t check_obstacle_x = 0;
  std::uint8_t check_obstacle_y = 0;

  const auto cell_is_free = [&](int x, int y)
  {
    return !arena.is_static_wall(x, y)
           && (arena.entity_at(x, y) == entt::null);
  };

  // Move the player.
  switch (direction)
    {
    case bim::game::player_action_kind::left:
      player.current_direction = bim::game::player_direction::left;

      if ((x_decimal <= half + offset)
          && ((x_int == 0) || !cell_is_free(x_int - 1, y_int)))
        position.x = x_floor + half;
      else
        {
          check_obstacle_x = x_int - 1;
          position.x -= offset;
        }
      break;
    case bim::game::player_action_kind::right:
      player.current_direction = bim::game::player_direction::right;

      if ((x_decimal + offset >= half)
          && ((x_int + 1 == arena.width()) || !cell_is_free(x_int + 1, y_int)))
        position.x = x_floor + half;
      else
        {
          check_obstacle_x = x_int + 1;
          position.x += offset;
        }
      break;
    case bim::game::player_action_kind::up:
      player.current_direction = bim::game::player_direction::up;

      if ((y_decimal <= half + offset)
          && ((y_int == 0) || !cell_is_free(x_int, y_int - 1)))
        position.y = y_floor + half;
      else
        {
          check_obstacle_y = y_int - 1;
          position.y -= offset;
        }
      break;
    case bim::game::player_action_kind::down:
      player.current_direction = bim::game::player_direction::down;

      if ((y_decimal + offset >= half)
          && ((y_int + 1 == arena.height())
              || !cell_is_free(x_int, y_int + 1)))
        position.y = y_floor + half;
      else
        {
          check_obstacle_y = y_int + 1;
          position.y += offset;
        }
      break;
    case bim::game::player_action_kind::drop_bomb:
      bim_assume(false);
      break;
    }

  // Adjust the position on the other axis than the movement to avoid
  // obstacles.
  if (check_obstacle_x != 0)
    {
      if (y_decimal > half)
        {
          if (!cell_is_free(check_obstacle_x, y_int + 1))
            position.y -= std::min(offset, y_decimal - half);
        }
      else if (y_decimal < half)
        {
          if (!cell_is_free(check_obstacle_x, y_int - 1))
            position.y += std::min(offset, half - y_decimal);
        }
    }
  else if (check_obstacle_y != 0)
    {
      if (x_decimal > half)
        {
          if (!cell_is_free(x_int + 1, check_obstacle_y))
            position.x -= std::min(offset, x_decimal - half);
        }
      else if (x_decimal < half)
        {
          if (!cell_is_free(x_int - 1, check_obstacle_y))
            position.x += std::min(offset, half - x_decimal);
        }
    }
}

static void
apply_player_actions(entt::registry& registry, bim::game::arena& arena,
                     bim::game::player& player,
                     bim::game::fractional_position_on_grid& position,
                     bim::game::player_action& action)
{
  for (std::uint8_t i = 0; i != action.queue_size; ++i)
    if (action.queue[i] == bim::game::player_action_kind::drop_bomb)
      drop_bomb(registry, arena, player, position);
    else
      move_player(player, position, action.queue[i], arena);

  action.queue_size = 0;
}

void bim::game::apply_player_action(entt::registry& registry, arena& arena)
{
  registry.view<player, fractional_position_on_grid, player_action>().each(
      [&registry, &arena](player& player,
                          fractional_position_on_grid& position,
                          player_action& action) -> void
      {
        apply_player_actions(registry, arena, player, position, action);
      });
}
