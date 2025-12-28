// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <cstdint>
#include <vector>

namespace bim::game
{
  class arena;
  struct position_on_grid;

  class navigation_check
  {
  public:
    navigation_check();
    ~navigation_check();

    bool reachable(arena& arena, std::uint8_t from_x, std::uint8_t from_y,
                   std::uint8_t to_x, std::uint8_t to_y);

  private:
    table_2d<bool> m_queued;
    std::vector<position_on_grid> m_pending;
  };
}
