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
#include <bm/game/contest.hpp>

#include <bm/game/component/player.hpp>
#include <bm/game/component/player_action.hpp>
#include <bm/game/component/player_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <bm/game/system/apply_player_action.hpp>
#include <bm/game/system/update_bombs.hpp>
#include <bm/game/system/update_brick_walls.hpp>
#include <bm/game/system/update_flames.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/level_generation.hpp>
#include <bm/game/random_generator.hpp>

constexpr std::chrono::milliseconds bm::game::contest::tick_interval;

bm::game::contest::contest(std::uint64_t seed,
                           std::uint8_t brick_wall_probability,
                           std::uint8_t player_count, std::uint8_t arena_width,
                           std::uint8_t arena_height)
  : m_arena(arena_width, arena_height)
{
  bm_assume(player_count > 0);

  const position_on_grid player_start_position[]
      = { position_on_grid(1, 1), position_on_grid(arena_width - 2, 1),
          position_on_grid(1, arena_height - 2),
          position_on_grid(arena_width - 2, arena_height - 2) };
  const int start_position_count = std::size(player_start_position);

  for (std::size_t i = 0; i != player_count; ++i)
    {
      entt::entity p = m_registry.create();
      const int start_position_index = i % start_position_count;
      m_registry.emplace<player>(p, i, player_direction::down, 2);
      m_registry.emplace<position_on_grid>(
          p, player_start_position[start_position_index]);
      m_registry.emplace<player_action>(p);
    }

  generate_basic_level_structure(m_arena);

  bm::game::random_generator random(seed);

  for (int i = 0; i != 10; ++i)
    random();

  insert_random_brick_walls(m_arena, m_registry, random,
                            brick_wall_probability);
}

void bm::game::contest::tick()
{
  apply_player_action(m_registry, m_arena);
  update_bombs(m_registry, m_arena, tick_interval);
  update_flames(m_registry, m_arena, tick_interval);
  update_brick_walls(m_registry, m_arena);
}

entt::registry& bm::game::contest::registry()
{
  return m_registry;
}

const entt::registry& bm::game::contest::registry() const
{
  return m_registry;
}

const bm::game::arena& bm::game::contest::arena() const
{
  return m_arena;
}

void bm::game::contest::arena(const bm::game::arena& a)
{
  m_arena = a;
}
