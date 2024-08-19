// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  constexpr int g_player_steps_per_cell = 16;

  void apply_player_action(entt::registry& registry, arena& arena);
}
