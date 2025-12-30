// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class entity_world_map;

  entt::entity crate_factory(entt::registry& registry,
                             entity_world_map& entity_map, std::uint8_t x,
                             std::uint8_t y);
}
