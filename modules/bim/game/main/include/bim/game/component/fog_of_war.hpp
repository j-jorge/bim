// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/cell_neighborhood_fwd.hpp>

namespace bim::game
{
  enum class fog_state
  {
    /// The fog is appearing, i.e. going toward full opacity.
    roll_in,

    /// The fog is there, nothing is happening.
    stable,

    /// The fog is disappearing but will come back soon.
    blown,

    /// The fog is coming back after being blown,
    restore,

    /// The fog is disappearing definitively.
    hiding
  };

  struct fog_of_war
  {
    static constexpr std::uint8_t full_opacity = 255;

    std::uint8_t player_index;
    std::uint8_t opacity;
    cell_neighborhood neighborhood;
    fog_state state;
  };
}
