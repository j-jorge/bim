// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;
  class context;
  class entity_world_map;

  constexpr int g_player_steps_per_cell = 16;

  void apply_player_action(const context& context, entt::registry& registry,
                           const arena& arena,
                           bim::game::entity_world_map& entity_map);
}
