// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <cstdint>

namespace bim::game
{
  struct bomb
  {
    std::chrono::milliseconds duration_until_explosion;
    std::uint8_t strength;
    std::uint8_t player_index;
  };
}
