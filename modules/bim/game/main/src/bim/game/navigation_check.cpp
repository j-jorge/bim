// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/component/position_on_grid.hpp>

bim::game::navigation_check::navigation_check() = default;
bim::game::navigation_check::~navigation_check() = default;

/**
 * Tells if a cell can be reached from another one, ignoring any entity on the
 * path. Only static geometry is considered.
 */
// TODO: check benchmark.
bool bim::game::navigation_check::reachable(const arena& arena,
                                            std::uint8_t from_x,
                                            std::uint8_t from_y,
                                            std::uint8_t to_x,
                                            std::uint8_t to_y)
{
  // Distance to the target.
  const auto distance = [=](int x, int y) -> int
    {
      return (x - to_x) * (x - to_x) + (y - to_y) * (y - to_y);
    };

  bool result = false;

  // We are going to process a cell. Return true if we must continue, false if
  // the scan is over.
  const auto enter = [=, &result](int x, int y) -> bool
    {
      result = (x == to_x) && (y == to_y);
      return !result;
    };

  // Push a cell in the queue. Returns true if and only if the cell has been
  // inserted. k is the index of the cell in m_queue.
  const auto visit = [this](position_on_grid, int nx, int ny,
                            std::size_t& k) -> bool
    {
      if (m_queued(nx, ny))
        return false;

      m_queued(nx, ny) = true;
      k = m_pending.size();

      m_pending.emplace_back(nx, ny);

      return true;
    };

  scan(arena, from_x, from_y, enter, visit, distance);

  return result;
}

// TODO: add benchmark.
void bim::game::navigation_check::paths(
    bim::table_2d<std::uint8_t>& distance,
    bim::table_2d<position_on_grid>& previous, const entt::registry& registry,
    const arena& arena, const entity_world_map& entity_map,
    std::uint8_t from_x, std::uint8_t from_y,
    const bim::table_2d<bool>& allowed)
{
  distance.fill(unreachable);
  distance(from_x, from_y) = 0;
  previous(from_x, from_y) = position_on_grid(from_x, from_y);

  // There's no pruning when searching the paths, so enter always returns true.
  const auto enter = [&allowed](int x, int y) -> bool
    {
      return true;
    };

  // Push a cell in the queue. Returns true if and only if the cell has been
  // inserted. p is the previous cell (where we were before reaching (nx,
  // ny)). k is the index of the cell in m_queue.
  const auto visit = [&, this](position_on_grid p, int nx, int ny,
                               std::size_t& k) -> bool
    {
      if (!allowed(nx, ny) || is_solid(registry, entity_map, nx, ny))
        {
          distance(nx, ny) = border;
          return false;
        }

      // Distance of (nx, ny) from the initial position.
      const int d = distance(p.x, p.y) + 1;

      if (m_queued(nx, ny))
        {
          // We already have the neighbor in the queue, just update its
          // distance.

          if (distance(nx, ny) < d)
            return false;

          distance(nx, ny) = d;
          previous(nx, ny) = p;

          // std::find will necessarily find (nx, ny) since it is in
          // m_queued.
          k = std::find(m_pending.begin(), m_pending.end(),
                        position_on_grid(nx, ny))
              - m_pending.begin();
        }
      else
        {
          // This is a new neighbor.
          distance(nx, ny) = d;
          previous(nx, ny) = p;

          k = m_pending.size();

          m_queued(nx, ny) = true;
          m_pending.emplace_back(nx, ny);
        }

      return true;
    };

  scan(arena, from_x, from_y, enter, visit, distance);
}

// TODO: add benchmark.
bool bim::game::navigation_check::exists(const entt::registry& registry,
                                         const arena& arena,
                                         const entity_world_map& entity_map,
                                         std::uint8_t from_x,
                                         std::uint8_t from_y, int max_distance,
                                         const bim::table_2d<bool>& forbidden)
{
  const auto distance = [=](int x, int y) -> int
    {
      return std::abs(x - from_x) + std::abs(y - from_y);
    };

  bool result = false;

  // We are going to process a cell. Return true if we can continue, false to
  // stop the scan now.
  const auto enter = [&](int x, int y) -> bool
    {
      result = !forbidden(x, y);

      // As soon as we have found a cell matching the criteria, we can leave.
      return !result;
    };

  // Push a cell in the queue. Returns true if and only if the cell has been
  // inserted. k is the index of the cell in m_queue.
  const auto visit = [&, this](position_on_grid, int nx, int ny,
                               std::size_t& k) -> bool
    {
      if (m_queued(nx, ny))
        return false;

      const int d = distance(nx, ny);

      if ((d > max_distance) || is_solid(registry, entity_map, nx, ny))
        return false;

      m_queued(nx, ny) = true;
      k = m_pending.size();

      m_pending.emplace_back(nx, ny);

      return true;
    };

  scan(arena, from_x, from_y, enter, visit, distance);

  return result;
}

template <typename Enter, typename Visit, typename Distance>
void bim::game::navigation_check::scan(const arena& arena, std::uint8_t from_x,
                                       std::uint8_t from_y, Enter&& enter,
                                       Visit&& visit, Distance&& distance)
{
  static constexpr int g_dx[] = { -1, 0, 0, 1 };
  static constexpr int g_dy[] = { 0, -1, 1, 0 };
  static constexpr bim::game::cell_edge g_edge[] = {
    bim::game::cell_edge::left, bim::game::cell_edge::up,
    bim::game::cell_edge::down, bim::game::cell_edge::right
  };
  static constexpr bim::game::cell_edge g_opposite_edge[] = {
    bim::game::cell_edge::right, bim::game::cell_edge::down,
    bim::game::cell_edge::up, bim::game::cell_edge::left
  };

  const int arena_width = arena.width();
  const int arena_height = arena.height();

  m_queued.resize(arena_width, arena_height);
  m_queued.fill(false);
  m_queued(from_x, from_y) = true;

  // m_pending is going to be sorted by decreasing distance from the start
  // position, such that the point the closest to the initial position is
  // popped from the end at each iteration.
  m_pending.clear();
  m_pending.reserve(arena_width * arena_height);
  m_pending.emplace_back(from_x, from_y);

  while (!m_pending.empty())
    {
      const position_on_grid p = m_pending.back();
      m_pending.pop_back();

      if (!enter(p.x, p.y))
        return;

      const cell_edge e = arena.fences(p.x, p.y);

      for (std::size_t i = 0; i != std::size(g_dx); ++i)
        {
          if (!!(e & g_edge[i]))
            continue;

          const int nx = p.x + g_dx[i];
          const int ny = p.y + g_dy[i];

          if ((nx < 0) || (ny < 0) || (nx >= arena_width)
              || (ny >= arena_height)
              || !!(arena.fences(nx, ny) & g_opposite_edge[i])
              || arena.is_static_wall(nx, ny))
            continue;

          std::size_t k;

          if (!visit(p, nx, ny, k))
            continue;

          const int d = distance(nx, ny);

          // Move (nx, ny) at its new position in m_pending.
          while (k != 0)
            {
              --k;

              if (distance(m_pending[k].x, m_pending[k].y) >= d)
                break;

              std::swap(m_pending[k + 1], m_pending[k]);
            }
        }
    }
}
