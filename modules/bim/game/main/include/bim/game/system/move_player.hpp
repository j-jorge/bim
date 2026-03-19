// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_movement_fwd.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;
  class entity_world_map;
  struct fractional_position_on_grid;

  void move_player(fractional_position_on_grid& position,
                   player_movement movement, const entt::registry& registry,
                   const arena& arena, const entity_world_map& entity_map);
}
