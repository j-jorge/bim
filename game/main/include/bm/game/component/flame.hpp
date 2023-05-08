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

#include <chrono>

namespace bm::game
{
  enum class flame_horizontal : bool
  {
    yes,
    no
  };

  enum class flame_vertical : bool
  {
    yes,
    no
  };

  enum class flame_end : bool
  {
    yes,
    no
  };

  struct flame
  {
    flame_horizontal horizontal;
    flame_vertical vertical;
    flame_end end;
    std::chrono::milliseconds time_to_live;
  };
}
