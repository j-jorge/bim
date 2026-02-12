// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/flame_direction_fwd.hpp>

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class context;

  entt::entity flame_factory(const context& context, entt::registry& registry,
                             std::uint8_t x, std::uint8_t y,
                             flame_direction direction, flame_segment segment);
}
