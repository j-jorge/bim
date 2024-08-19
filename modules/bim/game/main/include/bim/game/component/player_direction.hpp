// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_direction_fwd.hpp>

namespace bim::game
{
  enum class player_direction : std::uint8_t
  {
    up,
    down,
    left,
    right
  };
}
