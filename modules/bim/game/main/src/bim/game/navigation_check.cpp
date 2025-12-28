// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/navigation_check.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/component/position_on_grid.hpp>

bim::game::navigation_check::navigation_check() = default;
bim::game::navigation_check::~navigation_check() = default;

bool bim::game::navigation_check::reachable(arena& arena, std::uint8_t from_x,
                                            std::uint8_t from_y,
                                            std::uint8_t to_x,
                                            std::uint8_t to_y)
{
  const int arena_width = arena.width();
  const int arena_height = arena.height();
  const position_on_grid target(to_x, to_y);

  static constexpr int dx[] = { -1, 0, 0, 1 };
  static constexpr int dy[] = { 0, -1, 1, 0 };
  static constexpr cell_edge edge[] = { cell_edge::left, cell_edge::up,
                                        cell_edge::down, cell_edge::right };
  static constexpr cell_edge opposite_edge[] = {
    cell_edge::right, cell_edge::down, cell_edge::up, cell_edge::left
  };

  m_queued.resize(arena_width, arena_height);
  m_queued.fill(false);
  m_queued(from_x, from_y) = true;

  m_pending.clear();
  m_pending.reserve(arena_width * arena_height);
  m_pending.emplace_back(from_x, from_y);

  const auto sq_dist = [=](int x, int y) -> int
  {
    return (x - to_x) * (x - to_x) + (y - to_y) * (y - to_y);
  };

  while (!m_pending.empty())
    {
      const position_on_grid p = m_pending.back();
      m_pending.pop_back();

      if (p == target)
        return true;

      const cell_edge e = arena.fences(p.x, p.y);

      for (std::size_t i = 0; i != std::size(dx); ++i)
        {
          if (!!(e & edge[i]))
            continue;

          const int nx = p.x + dx[i];
          const int ny = p.y + dy[i];

          if ((nx < 0) || (ny < 0) || (nx >= arena_width)
              || (ny >= arena_height) || m_queued(nx, ny)
              || !!(arena.fences(nx, ny) & opposite_edge[i])
              || arena.is_static_wall(nx, ny))
            continue;

          m_queued(nx, ny) = true;

          std::size_t k = m_pending.size();

          m_pending.emplace_back(nx, ny);
          const int d = sq_dist(nx, ny);

          while (k != 0)
            {
              --k;

              if (sq_dist(m_pending[k].x, m_pending[k].y) >= d)
                break;
              else
                std::swap(m_pending[k + 1], m_pending[k]);
            }
        }
    }

  return false;
}
