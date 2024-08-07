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

#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/max_player_count.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace bim::net
{
  struct server_update
  {
    std::uint32_t from_tick;
    std::array<std::vector<bim::game::player_action>,
               bim::game::g_max_player_count>
        actions;
  };
}
