#pragma once

#include <bm/game/player_direction_fwd.hpp>

namespace bm
{
  namespace game
  {
    enum class player_direction : std::uint8_t
      {
        up,
        down,
        left,
        right
      };
  }
}
