// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  struct bomb
  {
    std::uint8_t strength;
    std::uint8_t player_index;
  };
}
