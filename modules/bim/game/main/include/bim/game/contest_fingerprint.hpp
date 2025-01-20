// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

namespace bim::game
{
  struct contest_fingerprint
  {
    std::uint64_t seed;
    feature_flags features;
    std::uint8_t player_count;
    std::uint8_t brick_wall_probability;
    std::uint8_t arena_width;
    std::uint8_t arena_height;
  };
}
