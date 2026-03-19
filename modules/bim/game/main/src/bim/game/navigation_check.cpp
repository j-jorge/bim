// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/is_solid.hpp>

#include <algorithm>

bim::game::navigation_check::navigation_check() = default;
bim::game::navigation_check::~navigation_check() = default;

enum class bim::game::navigation_check::scan_loop_policy
{
  allow,
  forbid
};

/**
 * Tells if a cell can be reached from another one, ignoring any entity on the
 * path. Only static geometry is considered.
 */
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
      m_queued(nx, ny) = true;
      k = m_pending.size();

      m_pending.emplace_back(nx, ny);

      return true;
    };

  scan<scan_loop_policy::forbid>(arena, from_x, from_y, enter, visit,
                                 distance);

  return result;
}

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
  const auto enter = [](int x, int y) -> bool
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

  scan<scan_loop_policy::allow>(arena, from_x, from_y, enter, visit, distance);
}

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
      const int d = distance(nx, ny);

      if ((d > max_distance) || is_solid(registry, entity_map, nx, ny))
        return false;

      m_queued(nx, ny) = true;
      k = m_pending.size();

      m_pending.emplace_back(nx, ny);

      return true;
    };

  scan<scan_loop_policy::forbid>(arena, from_x, from_y, enter, visit,
                                 distance);

  return result;
}

template <bim::game::navigation_check::scan_loop_policy ScanLoopPolicy,
          typename Enter, typename Visit, typename Distance>
void bim::game::navigation_check::scan(const arena& arena, std::uint8_t from_x,
                                       std::uint8_t from_y, Enter&& enter,
                                       Visit&& visit, Distance&& distance)
{
  static constexpr int dx[] = { 0, -1, 1, 0 };
  static constexpr int dy[] = { -1, 0, 0, 1 };
  static constexpr bim::game::cell_edge edge[] = {
    bim::game::cell_edge::up, bim::game::cell_edge::left,
    bim::game::cell_edge::right, bim::game::cell_edge::down
  };
  static constexpr bim::game::cell_edge opposite_edge[] = {
    bim::game::cell_edge::down, bim::game::cell_edge::right,
    bim::game::cell_edge::left, bim::game::cell_edge::up
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

  do
    {
      const position_on_grid p = m_pending.back();
      m_pending.pop_back();

      if (!enter(p.x, p.y))
        return;

      const cell_edge e = arena.fences(p.x, p.y);
      const bool no_edge[] = { !(e & edge[0]), !(e & edge[1]), !(e & edge[2]),
                               !(e & edge[3]) };
      const int neighbor_x[] = { p.x + dx[0], p.x + dx[1], p.x + dx[2],
                                 p.x + dx[3] };
      const int neighbor_y[] = { p.y + dy[0], p.y + dy[1], p.y + dy[2],
                                 p.y + dy[3] };
      const bool in_left[] = { neighbor_x[0] >= 0, neighbor_x[1] >= 0,
                               neighbor_x[2] >= 0, neighbor_x[3] >= 0 };
      const bool in_right[] = { neighbor_x[0] < arena_width,
                                neighbor_x[1] < arena_width,
                                neighbor_x[2] < arena_width,
                                neighbor_x[3] < arena_width };
      const bool in_up[] = { neighbor_y[0] >= 0, neighbor_y[1] >= 0,
                             neighbor_y[2] >= 0, neighbor_y[3] >= 0 };
      const bool in_down[] = { neighbor_y[0] < arena_height,
                               neighbor_y[1] < arena_height,
                               neighbor_y[2] < arena_height,
                               neighbor_y[3] < arena_height };
      const bool candidate[] = {
        (bool)(no_edge[0] & in_left[0] & in_right[0] & in_up[0] & in_down[0]),
        (bool)(no_edge[1] & in_left[1] & in_right[1] & in_up[1] & in_down[1]),
        (bool)(no_edge[2] & in_left[2] & in_right[2] & in_up[2] & in_down[2]),
        (bool)(no_edge[3] & in_left[3] & in_right[3] & in_up[3] & in_down[3])
      };

      for (std::size_t i = 0; i != std::size(dx); ++i)
        {
          if (!candidate[i])
            continue;

          const int nx = neighbor_x[i];
          const int ny = neighbor_y[i];

          if (((ScanLoopPolicy == scan_loop_policy::forbid)
               && m_queued(nx, ny))
              || !!(arena.fences(nx, ny) & opposite_edge[i])
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
  while (!m_pending.empty());
}
