// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  struct contest_fingerprint
  {
    std::uint64_t seed;
    std::uint32_t feature_mask;
    std::uint8_t player_count;
    std::uint8_t brick_wall_probability;
    std::uint8_t arena_width;
    std::uint8_t arena_height;
  };
}
