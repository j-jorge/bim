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
#include <bim/game/contest_runner.hpp>

#include <bim/game/check_game_over.hpp>

#include <bim/game/contest.hpp>

bim::game::contest_runner::contest_runner(contest& contest)
  : m_contest(contest)
{}

bim::game::contest_result
bim::game::contest_runner::run(std::chrono::nanoseconds elapsed_wall_time)
{
  for (int i = 0, n = m_tick_counter.add(elapsed_wall_time,
                                         bim::game::contest::tick_interval);
       i != n; ++i)
    {
      m_contest.tick();

      const contest_result result = check_game_over(m_contest.registry());

      if (!result.still_running())
        return result;
    }

  return contest_result::create_still_running();
}
