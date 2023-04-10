#pragma once

#include <bm/game/player_action.hpp>

namespace bm
{
  namespace game
  {
    struct player
    {
      std::uint8_t x;
      std::uint8_t y;

      player_direction current_direction;

      player_action pending_action;
    };
  }
}
