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
#include <bim/app/console/inputs.hpp>

#include <bim/game/player_action.hpp>

#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_kind.hpp>

#include <entt/entity/registry.hpp>

std::jthread bim::app::console::launch_input_thread(std::atomic<int>& input)
{
  return std::jthread(
      [&](std::stop_token stop_token) -> void
      {
        // getchar() is blocking, thus the thread is naturally waiting.
        while (!stop_token.stop_requested())
          input.store(std::getchar());
      });
}

bool bim::app::console::apply_inputs(entt::registry& registry,
                                     int player_index, int input)
{
  if (input == 'q')
    return false;

  bim::game::player_action* player_action =
      bim::game::find_player_action_by_index(registry, player_index);

  if ((player_action == nullptr) || player_action->full())
    return true;

  switch (input)
    {
      // Those case statements are configured to match the inputs obtained by
      // pressing the arrow keys on my laptop (ignoring the preceeding escape
      // character).
    case 'A':
      player_action->push(bim::game::player_action_kind::up);
      break;
    case 'B':
      player_action->push(bim::game::player_action_kind::down);
      break;
    case 'C':
      player_action->push(bim::game::player_action_kind::right);
      break;
    case 'D':
      player_action->push(bim::game::player_action_kind::left);
      break;
    case ' ':
      player_action->push(bim::game::player_action_kind::drop_bomb);
      break;
    }

  return true;
}
