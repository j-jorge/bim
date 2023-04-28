#include <bm/game/contest.hpp>

#include <bm/game/level_generation.hpp>
#include <bm/game/player.hpp>
#include <bm/game/player_direction.hpp>

#include <entt/entity/registry.hpp>

bm::game::contest::contest(entt::registry& registry, std::uint8_t player_count,
                           std::uint8_t arena_width, std::uint8_t arena_height)
  : m_arena(arena_width, arena_height)
{
  bm_assume(player_count > 0);

  const int player_start_position_x[] = { 1, arena_width - 1 };
  const int player_start_position_y[] = { 1, arena_height - 1 };
  const int start_position_count = std::size(player_start_position_x);
  assert(start_position_count == std::size(player_start_position_y));

  for(std::size_t i = 0; i != player_count; ++i)
    {
      entt::entity p = registry.create();
      const int start_position_index = i % start_position_count;
      registry.emplace<player>(p,
                               player_start_position_x[start_position_index],
                               player_start_position_y[start_position_index],
                               player_direction::down);
    }

  generate_basic_level(registry, m_arena);
}

void bm::game::contest::tick()
{
  // update_bombs(m_bombs, m_arena);
  // update_flames(m_flames, m_arena);
  // update_player_movement(m_players, m_arena);
  // check_player_collision(m_players, m_arena);
}

const bm::game::arena& bm::game::contest::arena() const
{
  return m_arena;
}
