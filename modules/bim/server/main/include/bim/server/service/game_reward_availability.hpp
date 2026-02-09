// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/game_reward_availability_fwd.hpp>

namespace bim::server
{
  enum class game_reward_availability : std::uint8_t
  {
    unavailable,
    available
  };
}
