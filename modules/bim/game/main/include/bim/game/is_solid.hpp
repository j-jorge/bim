// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class entity_world_map;

  bool is_solid(const entt::registry& registry,
                const entity_world_map& entity_map, std::uint8_t arena_x,
                std::uint8_t arena_y);
}
