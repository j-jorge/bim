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
