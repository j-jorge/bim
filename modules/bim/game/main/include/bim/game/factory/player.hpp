// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_id.hpp>

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class entity_world_map;

  entt::entity player_factory(entt::registry& registry,
                              entity_world_map& entity_map, std::uint8_t index,
                              std::uint8_t cell_x, std::uint8_t cell_y,
                              animation_id initial_state);
}
