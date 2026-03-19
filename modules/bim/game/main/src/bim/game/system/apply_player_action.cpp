// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/apply_player_action.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/bomb.hpp>
#include <bim/game/is_solid.hpp>
#include <bim/game/system/move_player.hpp>

#include <entt/entity/registry.hpp>

static void drop_bomb(entt::registry& registry,
                      bim::game::entity_world_map& entity_map,
                      bim::game::player& player, std::uint8_t arena_x,
                      std::uint8_t arena_y)
{
  if (player.bomb_available == 0)
    return;

  if (is_solid(registry, entity_map, arena_x, arena_y))
    return;

  --player.bomb_available;
  bim::game::bomb_factory(registry, entity_map, arena_x, arena_y,
                          player.bomb_strength, player.index);
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

static void move_player(entt::entity player_entity,
                        bim::game::fractional_position_on_grid& position,
                        bim::game::player_movement movement,
                        bim::game::animation_state& state,
                        const entt::registry& registry,
                        const bim::game::arena& arena,
                        bim::game::entity_world_map& entity_map,
                        const bim::game::player_animations& animations)
{
  if (movement == bim::game::player_movement::idle)
    return;

  using position_t = bim::game::fractional_position_on_grid::value_type;

  const position_t x = position.x;
  const position_t y = position.y;

  // Same as x_floor and y_floor but with a different type, for when we need
  // integers (e.g. cell coordinates in the arena).
  const std::uint8_t x_int = (std::uint8_t)position.x;
  const std::uint8_t y_int = (std::uint8_t)position.y;

  bim::game::move_player(position, movement, registry, arena, entity_map);

  if ((position.x == x) && (position.y == y))
    switch_to_idle_state(state, animations);
  else
    {
      const std::uint8_t new_x_int = (std::uint8_t)position.x;
      const std::uint8_t new_y_int = (std::uint8_t)position.y;

      entity_map.erase_entity(player_entity, x_int, y_int);
      entity_map.put_entity(player_entity, new_x_int, new_y_int);
    }
}

static void
apply_player_actions(entt::registry& registry, const bim::game::arena& arena,
                     bim::game::entity_world_map& entity_map,
                     const bim::game::player_animations& animations,
                     entt::entity player_entity, bim::game::player& player,
                     bim::game::fractional_position_on_grid& position,
                     const bim::game::queued_action& queued_action,
                     bim::game::animation_state& state)
{
  // Move the player.
  switch (queued_action.action.movement)
    {
    case bim::game::player_movement::left:
      state.transition_to(animations.walk_left);
      break;
    case bim::game::player_movement::right:
      state.transition_to(animations.walk_right);
      break;
    case bim::game::player_movement::up:
      state.transition_to(animations.walk_up);
      break;
    case bim::game::player_movement::down:
      state.transition_to(animations.walk_down);
      break;
    case bim::game::player_movement::idle:
      switch_to_idle_state(state, animations);
      break;
    }

  move_player(player_entity, position, queued_action.action.movement, state,
              registry, arena, entity_map, animations);

  if (queued_action.action.drop_bomb)
    drop_bomb(registry, entity_map, player, queued_action.arena_x,
              queued_action.arena_y);
}

void bim::game::apply_player_action(const context& context,
                                    entt::registry& registry,
                                    const arena& arena,
                                    bim::game::entity_world_map& entity_map)
{
  const player_animations& animations = context.get<const player_animations>();

  registry
      .view<player, fractional_position_on_grid, player_action,
            player_action_queue, animation_state>()
      .each(
          [&](entt::entity e, player& player,
              fractional_position_on_grid& position,
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
              apply_player_actions(registry, arena, entity_map, animations, e,
                                   player, position, action, state);
            });
}
