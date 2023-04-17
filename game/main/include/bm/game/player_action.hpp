#pragma once

#include <bm/game/player_direction_fwd.hpp>

namespace bm
{
  namespace game
  {
    struct player_action
    {
      player_direction _requested;
      bool bomb_drop;
    };
  }
}
