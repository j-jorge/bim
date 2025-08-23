// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/apply_player_action.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/factory/bomb.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

#include <fpm/math.hpp>

static void drop_bomb(entt::registry& registry, bim::game::arena& arena,
                      bim::game::player& player, std::uint8_t arena_x,
                      std::uint8_t arena_y)
{
  if (arena.entity_at(arena_x, arena_y) != entt::null)
    return;

  if (player.bomb_available == 0)
    return;

  --player.bomb_available;
  arena.put_entity(arena_x, arena_y,
                   bim::game::bomb_factory(registry, arena_x, arena_y,
                                           player.bomb_strength,
                                           player.index));
  arena.set_solid(arena_x, arena_y);
}

static void
switch_to_idle_state(bim::game::animation_state& state,
                     const bim::game::player_animations& animations)
{
  if (state.model == animations.walk_left)
    state.transition_to(animations.idle_left);
  else if (state.model == animations.walk_right)
    state.transition_to(animations.idle_right);
  else if (state.model == animations.walk_up)
    state.transition_to(animations.idle_up);
  else if (state.model == animations.walk_down)
    state.transition_to(animations.idle_down);
}

static void move_player(bim::game::player& player,
                        bim::game::fractional_position_on_grid& position,
                        bim::game::player_movement movement,
                        bim::game::animation_state& state,
                        const bim::game::arena& arena,
                        const bim::game::player_animations& animations)
{
  if (movement == bim::game::player_movement::idle)
    {
      switch_to_idle_state(state, animations);
      return;
    }

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

  // Move the player.
  switch (movement)
    {
    case bim::game::player_movement::left:
      state.transition_to(animations.walk_left);

      if ((x_decimal <= half + offset) && arena.is_blocker(x_int - 1, y_int))
        {
          position.x = x_floor + half;

          // turn around the solid block.
          if (y_decimal < half)
            {
              // There is a path above: move up.
              if (!arena.is_blocker(x_int, y_int - 1)
                  && !arena.is_blocker(x_int - 1, y_int - 1))
                position.y -= offset;
            }
          else if (y_decimal > half)
            {
              // There is a path below: move down.
              if (!arena.is_blocker(x_int, y_int + 1)
                  && !arena.is_blocker(x_int - 1, y_int + 1))
                position.y += offset;
            }
        }
      else
        {
          check_obstacle_x = x_int - 1;
          position.x -= offset;
        }
      break;
    case bim::game::player_movement::right:
      state.transition_to(animations.walk_right);

      if ((x_decimal + offset >= half) && arena.is_blocker(x_int + 1, y_int))
        {
          position.x = x_floor + half;

          // turn around the solid block.
          if (y_decimal < half)
            {
              // There is a path above: move up.
              if (!arena.is_blocker(x_int, y_int - 1)
                  && !arena.is_blocker(x_int + 1, y_int - 1))
                position.y -= offset;
            }
          else if (y_decimal > half)
            {
              // There is a path below: move down.
              if (!arena.is_blocker(x_int, y_int + 1)
                  && !arena.is_blocker(x_int + 1, y_int + 1))
                position.y += offset;
            }
        }
      else
        {
          check_obstacle_x = x_int + 1;
          position.x += offset;
        }
      break;
    case bim::game::player_movement::up:
      state.transition_to(animations.walk_up);

      if ((y_decimal <= half + offset) && arena.is_blocker(x_int, y_int - 1))
        {
          position.y = y_floor + half;

          // turn around the solid block.
          if (x_decimal < half)
            {
              // There is a path on the left: move left.
              if (!arena.is_blocker(x_int - 1, y_int)
                  && !arena.is_blocker(x_int - 1, y_int - 1))
                position.x -= offset;
            }
          else if (x_decimal > half)
            {
              // There is a path on the right: move right.
              if (!arena.is_blocker(x_int + 1, y_int)
                  && !arena.is_blocker(x_int + 1, y_int - 1))
                position.x += offset;
            }
        }
      else
        {
          check_obstacle_y = y_int - 1;
          position.y -= offset;
        }
      break;
    case bim::game::player_movement::down:
      state.transition_to(animations.walk_down);

      if ((y_decimal + offset >= half) && arena.is_blocker(x_int, y_int + 1))
        {
          position.y = y_floor + half;

          // turn around the solid block.
          if (x_decimal < half)
            {
              // There is a path on the left: move left.
              if (!arena.is_blocker(x_int - 1, y_int)
                  && !arena.is_blocker(x_int - 1, y_int + 1))
                position.x -= offset;
            }
          else if (x_decimal > half)
            {
              // There is a path on the right: move right.
              if (!arena.is_blocker(x_int + 1, y_int)
                  && !arena.is_blocker(x_int + 1, y_int + 1))
                position.x += offset;
            }
        }
      else
        {
          check_obstacle_y = y_int + 1;
          position.y += offset;
        }
      break;
    case bim::game::player_movement::idle:
      bim_assume(false);
      break;
    }

  // Adjust the position on the other axis than the movement to avoid
  // obstacles.
  if (check_obstacle_x != 0)
    {
      if (y_decimal > half)
        {
          if (arena.is_blocker(check_obstacle_x, y_int + 1))
            position.y -= std::min(offset, y_decimal - half);
        }
      else if (y_decimal < half)
        {
          if (arena.is_blocker(check_obstacle_x, y_int - 1))
            position.y += std::min(offset, half - y_decimal);
        }
    }
  else if (check_obstacle_y != 0)
    {
      if (x_decimal > half)
        {
          if (arena.is_blocker(x_int + 1, check_obstacle_y))
            position.x -= std::min(offset, x_decimal - half);
        }
      else if (x_decimal < half)
        {
          if (arena.is_blocker(x_int - 1, check_obstacle_y))
            position.x += std::min(offset, half - x_decimal);
        }
    }

  if ((position.x == x) && (position.y == y))
    switch_to_idle_state(state, animations);
}

static void
apply_player_actions(entt::registry& registry, bim::game::arena& arena,
                     const bim::game::player_animations& animations,
                     bim::game::player& player,
                     bim::game::fractional_position_on_grid& position,
                     const bim::game::queued_action& queued_action,
                     bim::game::animation_state& state)
{
  move_player(player, position, queued_action.action.movement, state, arena,
              animations);

  if (queued_action.action.drop_bomb)
    drop_bomb(registry, arena, player, queued_action.arena_x,
              queued_action.arena_y);
}

void bim::game::apply_player_action(const context& context,
                                    entt::registry& registry, arena& arena)
{
  const player_animations& animations = context.get<const player_animations>();

  registry
      .view<player, fractional_position_on_grid, player_action,
            player_action_queue, animation_state>()
      .each(
          [&registry, &arena,
           &animations](player& player, fractional_position_on_grid& position,
                        player_action& scheduled_action,
                        player_action_queue& action_queue,
                        animation_state& state) -> void
          {
            if (!animations.is_alive(state.model))
              {
                scheduled_action = {};
                return;
              }

            const queued_action action = action_queue.enqueue(
                scheduled_action, position.grid_aligned_x(),
                position.grid_aligned_y());

            scheduled_action = {};
            apply_player_actions(registry, arena, animations, player, position,
                                 action, state);
          });
}
