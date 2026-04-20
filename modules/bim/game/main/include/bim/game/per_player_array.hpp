// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/constant/max_player_count.hpp>

#include <array>

namespace bim::game
{
  template <typename T>
  using per_player_array = std::array<T, g_max_player_count>;
}
