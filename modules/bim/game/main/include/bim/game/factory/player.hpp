// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  entt::entity player_factory(entt::registry& registry, std::uint8_t index,
                              std::uint8_t cell_x, std::uint8_t cell_y);
}
