// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  struct player
  {
    std::uint8_t index;
    std::uint8_t bomb_capacity;
    std::uint8_t bomb_available;
    std::uint8_t bomb_strength;
    bool invisible;
  };
}
