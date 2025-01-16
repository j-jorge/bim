// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  struct arena_reduction_state
  {
    /**
     * Index of the position of the next block. The positions are computed
     * during the initialization and stored in the system.
     */
    std::uint16_t index_of_next_fall;
  };
}
