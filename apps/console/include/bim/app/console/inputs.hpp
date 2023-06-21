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

#include <atomic>
#include <thread>

namespace bim::app::console
{
  std::jthread launch_input_thread(std::atomic<int>& input);

  /// Return false if the player asked to quit.
  bool apply_inputs(entt::registry& registry, int player_index, int input);
}
