// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <vector>

namespace bim::game
{
  class arena;
  class position_on_grid;

  class arena_reduction
  {
  public:
    explicit arena_reduction(const arena& arena);
    ~arena_reduction();

    /**
     * Decrease the timer.
     * If zero, pop a block, increment index_of_next_fall, set
     * delay_until_next_fall.
     */
    void update(entt::registry& registry, arena& arena) const;

  private:
    std::vector<position_on_grid> m_fall_order;
  };
}
