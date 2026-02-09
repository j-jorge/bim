// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_result.hpp>

#include <cstdint>

namespace bim::net
{
  struct contest_result
  {
    bim::game::contest_result game_result;
    std::uint16_t coins_reward;
  };
}
