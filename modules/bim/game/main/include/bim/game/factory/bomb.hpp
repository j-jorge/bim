// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>
#include <cstdint>

namespace bim::game
{
  entt::entity bomb_factory(entt::registry& registry, std::uint8_t x,
                            std::uint8_t y, std::uint8_t strength,
                            std::uint8_t player_index);
  entt::entity
  bomb_factory(entt::registry& registry, std::uint8_t x, std::uint8_t y,
               std::uint8_t strength, std::uint8_t player_index,
               std::chrono::milliseconds duration_until_explosion);
}
