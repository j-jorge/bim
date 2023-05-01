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

#include <bm/game/level_generation.hpp>

bm::game::contest::contest(std::uint64_t seed,
                           std::uint8_t brick_wall_probability,
                           std::uint8_t player_count, std::uint8_t arena_width,
                           std::uint8_t arena_height)
  : m_random(seed)
  , m_arena(arena_width, arena_height)
{
  for(int i = 0; i != 10; ++i)
    m_random();

  bm_assume(player_count > 0);

  const int player_start_position_x[] = { 1, arena_width - 1 };
  const int player_start_position_y[] = { 1, arena_height - 1 };
  const int start_position_count = std::size(player_start_position_x);
  assert(start_position_count == std::size(player_start_position_y));

  for(std::size_t i = 0; i != player_count; ++i)
    {
      entt::entity p = m_registry.create();
      const int start_position_index = i % start_position_count;
      m_registry.emplace<player>(p,
                                 player_start_position_x[start_position_index],
                                 player_start_position_y[start_position_index],
                                 player_direction::down);
      m_registry.emplace<player_action>(p);
    }

  generate_basic_level_structure(m_arena);
  insert_random_brick_walls(m_arena, m_registry, m_random,
                            brick_wall_probability);
}

void bm::game::contest::tick()
{
  m_registry.view<player, player_action>().each(
      [this](player& p, const player_action& a) -> void
      {
        int x = p.x;
        int y = p.y;

        if(a.requested)
          {
            switch(*a.requested)
              {
              case player_direction::left:
                x -= 1;
                break;
              case player_direction::right:
                x += 1;
                break;
              case player_direction::up:
                y -= 1;
                break;
              case player_direction::down:
                y += 1;
                break;
              }

            if((x >= 0) && (y >= 0) && (x < m_arena.width())
               && (y < m_arena.height()) && !m_arena.is_static_wall(x, y)
               && (m_arena.entity_at(x, y) == entt::null))
              {
                p.x = x;
                p.y = y;
              }
          }
      });

  // update_bombs(m_bombs, m_arena);
  // update_flames(m_flames, m_arena);
  // update_player_movement(m_players, m_arena);
  // check_player_collision(m_players, m_arena);
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
