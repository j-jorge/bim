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

  constexpr position_t offset = position_t(1) / 8;
  constexpr position_t half = position_t(1) / 2;

  position_t x = position.x;
  position_t y = position.y;

  const position_t x_floor = fpm::floor(position.x);
  const position_t y_floor = fpm::floor(position.y);

  const position_t x_decimal = position.x - x_floor;
  const position_t y_decimal = position.y - y_floor;

  const std::uint8_t x_int = (std::uint8_t)x;
  const std::uint8_t y_int = (std::uint8_t)y;

  const auto cell_is_free = [&](int x, int y)
  {
    return !arena.is_static_wall(x, y)
           && (arena.entity_at(x, y) == entt::null);
  };

  switch (direction)
    {
    case bim::game::player_action_kind::left:
      player.current_direction = bim::game::player_direction::left;

      if ((x_decimal <= half + offset)
          && ((x_int == 0) || !cell_is_free(x_int - 1, y_int)))
        position.x = x_floor + half;
      else
        position.x -= offset;
      break;
    case bim::game::player_action_kind::right:
      player.current_direction = bim::game::player_direction::right;

      if ((x_decimal + offset >= half)
          && ((x_int + 1 == arena.width()) || !cell_is_free(x_int + 1, y_int)))
        position.x = x_floor + half;
      else
        position.x += offset;
      break;
    case bim::game::player_action_kind::up:
      player.current_direction = bim::game::player_direction::up;

      if ((y_decimal <= half + offset)
          && ((y_int == 0) || !cell_is_free(x_int, y_int - 1)))
        position.y = y_floor + half;
      else
        position.y -= offset;
      break;
    case bim::game::player_action_kind::down:
      player.current_direction = bim::game::player_direction::down;

      if ((y_decimal + offset >= half)
          && ((y_int + 1 == arena.height())
              || !cell_is_free(x_int, y_int + 1)))
        position.y = y_floor + half;
      else
        position.y += offset;
      break;
    case bim::game::player_action_kind::drop_bomb:
      bim_assume(false);
      break;
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
