// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

namespace bim::game
{
  enum class feature_flags : std::uint32_t
  {
    falling_blocks = (1 << 0)
  };
}
