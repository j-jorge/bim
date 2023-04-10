#include <bm/game/contest.hpp>

static void update_bombs(std::span<bm::game::bomb> bombs, bm::game::arena& arena) {}
static void update_flames(std::span<bm::game::flame> flames, bm::game::arena& arena) {}
static void update_player_movement(std::span<bm::game::player> players, bm::game::arena& arena) {}
static void check_player_collision(std::span<bm::game::player> players, bm::game::arena& arena) {}

bm::game::contest::contest() = default;

bm::game::contest::contest(int player_count, int arena_width, int arena_height)
  : m_arena(arena_width, arena_height),
    m_players(player_count)
{}

void bm::game::contest::tick()
{
  update_bombs(m_bombs, m_arena);
  update_flames(m_flames, m_arena);
  update_player_movement(m_players, m_arena);
  check_player_collision(m_players, m_arena);
}

const bm::game::arena& bm::game::contest::arena() const
{
  return m_arena;
}

std::span<const bm::game::player> bm::game::contest::players() const
{
  return m_players;
}

std::span<const bm::game::flame> bm::game::contest::flames() const
{
  return m_flames;
}

std::span<const bm::game::bomb> bm::game::contest::bombs() const
{
  return m_bombs;
}

