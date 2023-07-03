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
#include <bim/game/tick_counter.hpp>

bim::game::tick_counter::tick_counter()
  : m_remainder(0)
{}

int bim::game::tick_counter::add(std::chrono::nanoseconds elapsed_time,
                                 std::chrono::nanoseconds time_step)
{
  m_remainder += elapsed_time;

  const int result = (m_remainder / time_step);
  m_remainder -= result * time_step;

  return result;
}
