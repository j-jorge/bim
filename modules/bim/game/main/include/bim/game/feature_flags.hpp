// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

namespace bim::game
{
  // Prefer to add features at the end (i.e. do not reorder the enum
  // values). This will save us from special cases when the features were
  // serialized before the addition of the new value, for example in the user's
  // preferences.
  enum class feature_flags : std::uint32_t
  {
    falling_blocks = (1 << 0),
    fog_of_war = (1 << 1),
    invisibility = (1 << 2),
    shield = (1 << 3),
    fences = (1 << 4)
  };
}
