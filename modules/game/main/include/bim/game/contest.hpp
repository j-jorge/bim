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

#include <bim/game/arena.hpp>

#include <entt/entity/registry.hpp>

#include <chrono>

namespace bim::game
{
  class contest
  {
  public:
    static constexpr std::chrono::milliseconds tick_interval
        = std::chrono::milliseconds(20);

  public:
    contest(std::uint64_t seed, std::uint8_t brick_wall_probability,
            std::uint8_t player_count, std::uint8_t arena_width,
            std::uint8_t arena_height);

    void tick();

    entt::registry& registry();
    const entt::registry& registry() const;
    const bim::game::arena& arena() const;
    void arena(const bim::game::arena& a);

  private:
    entt::registry m_registry;
    bim::game::arena m_arena;
  };
}
