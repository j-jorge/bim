// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest.hpp>

#include <bim/game/check_game_over.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/apply_player_action.hpp>
#include <bim/game/system/refresh_bomb_inventory.hpp>
#include <bim/game/system/remove_dead_objects.hpp>
#include <bim/game/system/update_bomb_power_up_spawners.hpp>
#include <bim/game/system/update_bomb_power_ups.hpp>
#include <bim/game/system/update_bombs.hpp>
#include <bim/game/system/update_brick_walls.hpp>
#include <bim/game/system/update_flame_power_up_spawners.hpp>
#include <bim/game/system/update_flame_power_ups.hpp>
#include <bim/game/system/update_flames.hpp>
#include <bim/game/system/update_players.hpp>

#include <bim/game/level_generation.hpp>
#include <bim/game/random_generator.hpp>

#include <bim/assume.hpp>

constexpr std::chrono::milliseconds bim::game::contest::tick_interval;

bim::game::contest::contest(std::uint64_t seed,
                            std::uint8_t brick_wall_probability,
                            std::uint8_t player_count,
                            std::uint8_t arena_width,
                            std::uint8_t arena_height)
  : m_arena(arena_width, arena_height)
{
  bim_assume(player_count > 0);

  const position_on_grid player_start_position[] = {
    position_on_grid(1, 1),
    position_on_grid(arena_width - 2, arena_height - 2),
    position_on_grid(arena_width - 2, 1), position_on_grid(1, arena_height - 2)
  };
  const int start_position_count = std::size(player_start_position);

  for (std::size_t i = 0; i != player_count; ++i)
    {
      const int start_position_index = i % start_position_count;
      player_factory(m_registry, i,
                     player_start_position[start_position_index].x,
                     player_start_position[start_position_index].y);
    }

  generate_basic_level_structure(m_arena);

  bim::game::random_generator random(seed);

  for (int i = 0; i != 10; ++i)
    random();

  insert_random_brick_walls(m_arena, m_registry, random,
                            brick_wall_probability);
}

bim::game::contest_result bim::game::contest::tick()
{
  refresh_bomb_inventory(m_registry);
  apply_player_action(m_registry, m_arena);
  update_bombs(m_registry, m_arena, tick_interval);
  update_flames(m_registry, m_arena, tick_interval);
  update_brick_walls(m_registry, m_arena);
  update_bomb_power_up_spawners(m_registry, m_arena);
  update_bomb_power_ups(m_registry, m_arena);
  update_flame_power_up_spawners(m_registry, m_arena);
  update_flame_power_ups(m_registry, m_arena);
  update_players(m_registry, m_arena);

  remove_dead_objects(m_registry);
  const contest_result result = check_game_over(m_registry);

  if (!result.still_running())
    return result;

  return result;
}

entt::registry& bim::game::contest::registry()
{
  return m_registry;
}

const entt::registry& bim::game::contest::registry() const
{
  return m_registry;
}

const bim::game::arena& bim::game::contest::arena() const
{
  return m_arena;
}

void bim::game::contest::arena(const bim::game::arena& a)
{
  m_arena = a;
}
