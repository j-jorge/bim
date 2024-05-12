/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class arena;
  class random_generator;

  constexpr std::uint8_t g_flame_power_up_count_in_level = 8;
  constexpr std::uint8_t g_bomb_power_up_count_in_level = 10;

  void generate_basic_level_structure(arena& arena);
  void insert_random_brick_walls(arena& arena, entt::registry& registry,
                                 random_generator& random_generator,
                                 std::uint8_t brick_wall_probability);
}
