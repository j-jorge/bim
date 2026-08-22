// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_death_kind_fwd.hpp>

namespace bim::game
{
  struct player_death_marker
  {
    std::uint8_t player_index;
    player_death_kind death_kind;
  };
}
