// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class arena;
  class random_generator;

  constexpr std::uint8_t g_flame_power_up_count_in_level = 8;
  constexpr std::uint8_t g_bomb_power_up_count_in_level = 10;
  constexpr std::uint8_t g_invisibility_power_up_count_in_level = 6;

  void generate_basic_level_structure(arena& arena);
  void insert_random_brick_walls(arena& arena, entt::registry& registry,
                                 random_generator& random_generator,
                                 std::uint8_t brick_wall_probability,
                                 bool invisibility_power_up_enabled = false);
}
