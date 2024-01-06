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
#include <bim/game/component/player_action_kind.hpp>

#include <bim/assume.hpp>

const char* bim::game::to_string(player_action_kind k)
{
  switch (k)
    {
    case player_action_kind::up:
      return "up";
    case player_action_kind::down:
      return "down";
    case player_action_kind::left:
      return "left";
    case player_action_kind::right:
      return "right";
    case player_action_kind::drop_bomb:
      return "drop_bomb";
    };

  bim_assume(false);
}
