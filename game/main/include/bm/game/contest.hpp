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

#include <bm/game/arena.hpp>
#include <bm/game/random_generator.hpp>

#include <entt/entity/registry.hpp>

namespace bm::game
{
  class contest
  {
  public:
    contest(std::uint64_t seed, std::uint8_t brick_wall_probability,
            std::uint8_t player_count, std::uint8_t arena_width,
            std::uint8_t arena_height);

    void tick();

    entt::registry& registry();
    const entt::registry& registry() const;
    const bm::game::arena& arena() const;

  private:
    entt::registry m_registry;
    bm::game::random_generator m_random;
    bm::game::arena m_arena;
  };
}
