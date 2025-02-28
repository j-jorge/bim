// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <span>

namespace bim::game
{
  struct position_on_grid;
  void fog_of_war_factory(entt::registry& registry, std::uint8_t player_index,
                          int arena_width, int arena_height,
                          const std::span<const position_on_grid>& exclude);
}
