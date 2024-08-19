// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/flame_direction_fwd.hpp>

namespace bim::game
{
  enum class flame_direction : std::uint8_t
  {
    right,
    down,
    left,
    up
  };

  enum class flame_segment : std::uint8_t
  {
    tip,
    arm,
    origin
  };
}
