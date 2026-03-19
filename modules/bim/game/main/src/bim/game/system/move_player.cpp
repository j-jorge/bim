// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/move_player.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/constant/player_steps_per_cell.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/is_solid.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

#include <fpm/math.hpp>

using position_t = bim::game::fractional_position_on_grid::value_type;

static constexpr position_t g_one(1);
static constexpr position_t g_half = g_one / 2;
static constexpr position_t g_step =
    g_one / bim::game::g_player_steps_per_cell;
static constexpr position_t g_edge_width = 2 * g_step;

static bim::game::cell_edge
edges_at(const entt::registry& registry, const bim::game::arena& arena,
         const bim::game::entity_world_map& entity_map, std::uint8_t arena_x,
         std::uint8_t arena_y)
{
  if (arena.is_static_wall(arena_x, arena_y)
      || is_solid(registry, entity_map, arena_x, arena_y))
    return bim::game::cell_edge::all;

  return arena.fences(arena_x, arena_y);
}

/**
 * Adjust the position on the other axis than the movement to avoid obstacles.
 */
static void straighten_horizontal_move(
    const entt::registry& registry, const bim::game::arena& arena,
    const bim::game::entity_world_map& entity_map,
    bim::game::fractional_position_on_grid& position, position_t y_decimal,
    std::uint8_t x, std::uint8_t y, bim::game::cell_edge edge)
{
  if (y_decimal > g_half)
    {
      if (!!(edges_at(registry, arena, entity_map, x, y + 1) & edge))
        position.y -= std::min(g_step, y_decimal - g_half);
    }
  else if (y_decimal < g_half)
    {
      if (!!(edges_at(registry, arena, entity_map, x, y - 1) & edge))
        position.y += std::min(g_step, g_half - y_decimal);
    }
}

/**
 * Adjust the position on the other axis than the movement to avoid obstacles.
 */
static void straighten_vertical_move(
    const entt::registry& registry, const bim::game::arena& arena,
    const bim::game::entity_world_map& entity_map,
    bim::game::fractional_position_on_grid& position, position_t x_decimal,
    std::uint8_t x, std::uint8_t y, bim::game::cell_edge edge)
{
  if (x_decimal > g_half)
    {
      if (!!(edges_at(registry, arena, entity_map, x + 1, y) & edge))
        position.x -= std::min(g_step, x_decimal - g_half);
    }
  else if (x_decimal < g_half)
    {
      if (!!(edges_at(registry, arena, entity_map, x - 1, y) & edge))
        position.x += std::min(g_step, g_half - x_decimal);
    }
}

static void dodge_vertical_move(
    const entt::registry& registry, const bim::game::arena& arena,
    const bim::game::entity_world_map& entity_map,
    bim::game::fractional_position_on_grid& position, position_t x_decimal,
    std::uint8_t x, std::uint8_t y, int dy, bim::game::cell_edge current_edges,
    bim::game::cell_edge edge_forward, bim::game::cell_edge edge_backward)
{
  if (x_decimal < g_half)
    {
      // The player is in the left half of the current cell, let's check if
      // there's a path forward-left.
      if (!(current_edges & bim::game::cell_edge::left)
          && !(edges_at(registry, arena, entity_map, x - 1, y)
               & (edge_forward | bim::game::cell_edge::right))
          && !(edges_at(registry, arena, entity_map, x - 1, y + dy)
               & edge_backward))
        position.x -= g_step;
    }
  else if (x_decimal > g_half)
    {
      // The player is in the right half of the current cell, let's check if
      // there's a path forward-right.
      if (!(current_edges & bim::game::cell_edge::right)
          && !(edges_at(registry, arena, entity_map, x + 1, y)
               & (edge_forward | bim::game::cell_edge::left))
          && !(edges_at(registry, arena, entity_map, x + 1, y + dy)
               & edge_backward))
        position.x += g_step;
    }
}

static void dodge_horizontal_move(
    const entt::registry& registry, const bim::game::arena& arena,
    const bim::game::entity_world_map& entity_map,
    bim::game::fractional_position_on_grid& position, position_t y_decimal,
    std::uint8_t x, std::uint8_t y, int dx, bim::game::cell_edge current_edges,
    bim::game::cell_edge edge_forward, bim::game::cell_edge edge_backward)
{
  if (y_decimal < g_half)
    {
      // The player is in the top half of the current cell, let's check if
      // there's a path up-forward.
      if (!(current_edges & bim::game::cell_edge::up)
          && !(edges_at(registry, arena, entity_map, x, y - 1)
               & (bim::game::cell_edge::up | edge_forward))
          && !(edges_at(registry, arena, entity_map, x + dx, y - 1)
               & edge_backward))
        position.y -= g_step;
    }
  else if (y_decimal > g_half)
    {
      // The player is in the bottom half of the current cell, let's check if
      // there's a path down-forward.
      if (!(current_edges & bim::game::cell_edge::down)
          && !(edges_at(registry, arena, entity_map, x, y + 1)
               & (bim::game::cell_edge::up | edge_forward))
          && !(edges_at(registry, arena, entity_map, x + dx, y + 1)
               & edge_backward))
        position.y += g_step;
    }
}

static void move_right(const entt::registry& registry,
                       const bim::game::arena& arena,
                       const bim::game::entity_world_map& entity_map,
                       std::uint8_t x_int, std::uint8_t y_int,
                       position_t x_decimal, position_t y_decimal,
                       position_t x_floor,
                       bim::game::fractional_position_on_grid& position)
{
  const bim::game::cell_edge edges = arena.fences(x_int, y_int);

  if ((x_decimal + g_step >= g_half - g_edge_width)
      && !!(edges & bim::game::cell_edge::right))
    // In contact with an edge on the right, inside the current cell.
    position.x = x_floor + g_half - g_edge_width;
  else if ((x_decimal + g_step >= g_half)
           && !!(edges_at(registry, arena, entity_map, x_int + 1, y_int)
                 & bim::game::cell_edge::left))
    // In contact with something solid inside the adjacent cell.
    position.x = x_floor + g_half;
  else
    {
      position.x += g_step;
      straighten_horizontal_move(registry, arena, entity_map, position,
                                 y_decimal, x_int + 1, y_int,
                                 bim::game::cell_edge::left);
      return;
    }

  // We are encountering an obstacle, try to dodge it.
  dodge_horizontal_move(registry, arena, entity_map, position, y_decimal,
                        x_int, y_int, 1, edges, bim::game::cell_edge::right,
                        bim::game::cell_edge::left);
}

static void
move_left(const entt::registry& registry, const bim::game::arena& arena,
          const bim::game::entity_world_map& entity_map, std::uint8_t x_int,
          std::uint8_t y_int, position_t x_decimal, position_t y_decimal,
          position_t x_floor, bim::game::fractional_position_on_grid& position)
{
  const bim::game::cell_edge edges = arena.fences(x_int, y_int);

  if ((x_decimal - g_step <= g_half + g_edge_width)
      && !!(edges & bim::game::cell_edge::left))
    // In contact with an edge on the left, inside the current cell.
    position.x = x_floor + g_half + g_edge_width;
  else if ((x_decimal - g_step <= g_half)
           && !!(edges_at(registry, arena, entity_map, x_int - 1, y_int)
                 & bim::game::cell_edge::right))
    // In contact with something solid inside the adjacent cell.
    position.x = x_floor + g_half;
  else
    {
      position.x -= g_step;
      straighten_horizontal_move(registry, arena, entity_map, position,
                                 y_decimal, x_int - 1, y_int,
                                 bim::game::cell_edge::right);
      return;
    }

  // We are encountering an obstacle, try to dodge it.
  dodge_horizontal_move(registry, arena, entity_map, position, y_decimal,
                        x_int, y_int, -1, edges, bim::game::cell_edge::left,
                        bim::game::cell_edge::right);
}

static void
move_up(const entt::registry& registry, const bim::game::arena& arena,
        const bim::game::entity_world_map& entity_map, std::uint8_t x_int,
        std::uint8_t y_int, position_t x_decimal, position_t y_decimal,
        position_t y_floor, bim::game::fractional_position_on_grid& position)
{
  const bim::game::cell_edge edges = arena.fences(x_int, y_int);

  if ((y_decimal - g_step <= g_half + g_edge_width)
      && !!(edges & bim::game::cell_edge::up))
    // In contact with an edge at the top, inside the current cell.
    position.y = y_floor + g_half + g_edge_width;
  else if ((y_decimal - g_step <= g_half)
           && !!(edges_at(registry, arena, entity_map, x_int, y_int - 1)
                 & bim::game::cell_edge::down))
    // In contact with something solid inside the adjacent cell.
    position.y = y_floor + g_half;
  else
    {
      position.y -= g_step;
      straighten_vertical_move(registry, arena, entity_map, position,
                               x_decimal, x_int, y_int - 1,
                               bim::game::cell_edge::down);
      return;
    }

  // We are encountering an obstacle, try to dodge it.
  dodge_vertical_move(registry, arena, entity_map, position, x_decimal, x_int,
                      y_int, -1, edges, bim::game::cell_edge::up,
                      bim::game::cell_edge::down);
}

static void
move_down(const entt::registry& registry, const bim::game::arena& arena,
          const bim::game::entity_world_map& entity_map, std::uint8_t x_int,
          std::uint8_t y_int, position_t x_decimal, position_t y_decimal,
          position_t y_floor, bim::game::fractional_position_on_grid& position)
{
  const bim::game::cell_edge edges = arena.fences(x_int, y_int);

  if ((y_decimal + g_step >= g_half - g_edge_width)
      && !!(edges & bim::game::cell_edge::down))
    // In contact with an edge at the bottom, inside the current cell.
    position.y = y_floor + g_half - g_edge_width;
  else if ((y_decimal + g_step >= g_half)
           && !!(edges_at(registry, arena, entity_map, x_int, y_int + 1)
                 & bim::game::cell_edge::up))
    // In contact with something solid inside the adjacent cell.
    position.y = y_floor + g_half;
  else
    {
      position.y += g_step;
      straighten_vertical_move(registry, arena, entity_map, position,
                               x_decimal, x_int, y_int + 1,
                               bim::game::cell_edge::up);
      return;
    }

  // We are encountering an obstacle, try to dodge it.
  dodge_vertical_move(registry, arena, entity_map, position, x_decimal, x_int,
                      y_int, 1, edges, bim::game::cell_edge::down,
                      bim::game::cell_edge::up);
}

void bim::game::move_player(fractional_position_on_grid& position,
                            player_movement movement,
                            const entt::registry& registry, const arena& arena,
                            const entity_world_map& entity_map)
{
  if (movement == player_movement::idle)
    return;

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

  // Move the player.
  switch (movement)
    {
    case player_movement::left:
      move_left(registry, arena, entity_map, x_int, y_int, x_decimal,
                y_decimal, x_floor, position);
      break;
    case player_movement::right:
      move_right(registry, arena, entity_map, x_int, y_int, x_decimal,
                 y_decimal, x_floor, position);
      break;
    case player_movement::up:
      move_up(registry, arena, entity_map, x_int, y_int, x_decimal, y_decimal,
              y_floor, position);
      break;
    case player_movement::down:
      move_down(registry, arena, entity_map, x_int, y_int, x_decimal,
                y_decimal, y_floor, position);
      break;
    case player_movement::idle:
      bim_assume(false);
      break;
    }
}
