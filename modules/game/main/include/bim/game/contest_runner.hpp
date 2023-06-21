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

#include <bim/game/tick_counter.hpp>

namespace bim::game
{
  class contest;

  class contest_runner
  {
  public:
    explicit contest_runner(contest& contest);

    void run(std::chrono::nanoseconds elapsed_wall_time);

  private:
    contest& m_contest;
    tick_counter m_tick_counter;
  };
}
