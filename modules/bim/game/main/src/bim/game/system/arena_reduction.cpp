// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/arena_reduction.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/arena_reduction_state.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/falling_block_duration.hpp>
#include <bim/game/factory/falling_block.hpp>

#include <entt/entity/registry.hpp>

bim::game::arena_reduction::arena_reduction(const arena& arena)
{
  const int width = arena.width();
  const int height = arena.height();
  std::vector<std::uint8_t> availability(width * height);
  std::size_t free_count = 0;

  for (int y = 0; y != height; ++y)
    for (int x = 0; x != width; ++x)
      {
        const bool is_free = !arena.is_static_wall(x, y);
        free_count += is_free;
        availability[y * width + x] = is_free;
      }

  m_fall_order.reserve(free_count);

  int horizontal_first = 0;
  int horizontal_last = width - 1;
  int vertical_first = 0;
  int vertical_last = height - 1;

  const auto horizontal_scan = [this, width, &availability](int y, int first,
                                                            int last) -> int
  {
    const int inc = (last < first) ? -1 : 1;
    const int n = inc * (last - first) + 1;

    for (int i = 0; i != n; ++i, first += inc)
      if (availability[y * width + first])
        {
          m_fall_order.emplace_back(first, y);
          availability[y * width + first] = false;
          return 1;
        }

    return 0;
  };

  const auto vertical_scan = [this, width, &availability](int x, int first,
                                                          int last) -> int
  {
    const int inc = (last < first) ? -1 : 1;
    const int n = inc * (last - first) + 1;

    for (int i = 0; i != n; ++i, first += inc)
      if (availability[first * width + x])
        {
          m_fall_order.emplace_back(x, first);
          availability[first * width + x] = false;
          return 1;
        }

    return 0;
  };

  while ((vertical_first <= vertical_last)
         && (horizontal_first <= horizontal_last))
    {
      int added_count =
          horizontal_scan(vertical_first, horizontal_first, horizontal_last);
      added_count +=
          vertical_scan(horizontal_last, vertical_first, vertical_last);
      added_count +=
          horizontal_scan(vertical_last, horizontal_last, horizontal_first);
      added_count +=
          vertical_scan(horizontal_first, vertical_last, vertical_first);

      if (added_count == 0)
        {
          ++vertical_first;
          --vertical_last;
          ++horizontal_first;
          --horizontal_last;
        }
    }
}

bim::game::arena_reduction::~arena_reduction() = default;

void bim::game::arena_reduction::update(entt::registry& registry,
                                        arena& arena) const
{
  registry.view<arena_reduction_state, timer>().each(
      [&, this](arena_reduction_state& state, timer& t) -> void
      {
        if (t.duration.count() > 0)
          return;

        if (state.index_of_next_fall >= m_fall_order.size())
          return;

        t.duration = g_falling_block_duration;
        falling_block_factory(registry, m_fall_order[state.index_of_next_fall],
                              g_falling_block_duration);
        ++state.index_of_next_fall;
      });
}
