#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bm::game
{
  class arena;
  class random_generator;

  void generate_basic_level_structure(arena& arena);
  void insert_random_brick_walls(arena& arena, entt::registry& registry,
                                 random_generator& random_generator,
                                 std::uint8_t brick_wall_probability);
}
