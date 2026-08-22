// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_death_kind_fwd.hpp>

namespace bim::game
{
  enum class player_death_kind : std::uint8_t
  {
    defeated,
    kicked
  };
}
