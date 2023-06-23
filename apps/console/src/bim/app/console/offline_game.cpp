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
#include <bim/app/console/offline_game.hpp>

#include <bim/app/console/application.hpp>
#include <bim/app/console/display.hpp>
#include <bim/app/console/inputs.hpp>

#include <iscool/schedule/delayed_call.hpp>

#include <random>

bim::app::console::offline_game::offline_game(application& application)
  : m_application(application)
  , m_contest(std::random_device()(), 80, 4, 13, 11)
  , m_contest_runner(m_contest)
  , m_input_thread(launch_input_thread(m_input))

{
  schedule_tick();
}

void bim::app::console::offline_game::schedule_tick()
{
  m_tick_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        tick();
      },
      m_application.update_interval());
}

void bim::app::console::offline_game::tick()
{
  if (!apply_inputs(m_contest.registry(), 0, m_input.exchange(0)))
    m_application.quit();
  else
    {
      m_contest_runner.run(m_application.update_interval());
      display(m_contest);
      schedule_tick();
    }
}
