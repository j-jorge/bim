#pragma once

#include <bm/game/player_direction_fwd.hpp>

#include <optional>

namespace bm
{
  namespace game
  {
    struct player_action
    {
      std::optional<player_direction> requested;
      bool drop_bomb;
    };
  }
}
