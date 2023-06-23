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

#include <bim/game/contest.hpp>
#include <bim/game/contest_runner.hpp>

#include <iscool/signals/scoped_connection.hpp>

#include <atomic>
#include <thread>

namespace bim::app::console
{
  class application;

  class offline_game
  {
  public:
    explicit offline_game(application& application);

  private:
    void schedule_tick();
    void tick();

  private:
    application& m_application;

    bim::game::contest m_contest;
    bim::game::contest_runner m_contest_runner;

    std::atomic<int> m_input;
    std::jthread m_input_thread;

    iscool::signals::scoped_connection m_tick_connection;
  };
}
