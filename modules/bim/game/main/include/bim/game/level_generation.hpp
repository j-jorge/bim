// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags.hpp>

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class arena;
  class random_generator;

  constexpr std::uint8_t g_flame_power_up_count_in_level = 8;
  constexpr std::uint8_t g_bomb_power_up_count_in_level = 10;
  constexpr std::uint8_t g_invisibility_power_up_count_in_level = 4;
  constexpr std::uint8_t g_shield_power_up_count_in_level = 4;

  void generate_basic_level_structure(arena& arena);
  void insert_random_crates(arena& arena, entt::registry& registry,
                                 random_generator& random_generator,
                                 std::uint8_t crate_probability,
                                 feature_flags features);
  bool valid_invisibility_power_up_position(int x, int y, int w, int h);
}
