// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/flame_direction_fwd.hpp>

#include <entt/entity/fwd.hpp>

#include <chrono>
#include <cstdint>

namespace bim::game
{
  entt::entity flame_factory(entt::registry& registry, std::uint8_t x,
                             std::uint8_t y, flame_direction direction,
                             flame_segment segment);
  entt::entity flame_factory(entt::registry& registry, std::uint8_t x,
                             std::uint8_t y, flame_direction direction,
                             flame_segment segment,
                             std::chrono::milliseconds time_to_live);
}
