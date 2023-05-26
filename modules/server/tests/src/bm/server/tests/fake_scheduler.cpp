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
#include <bm/server/tests/fake_scheduler.hpp>

#include <thread>

bm::server::tests::fake_scheduler::fake_scheduler()
  : m_current_date{}
  , m_time_source_initializer(
        [this]() -> std::chrono::nanoseconds
        {
          return m_current_date;
        })
  , m_scheduler_initializer(m_scheduler.get_delayed_call_delegate())
{}

void bm::server::tests::fake_scheduler::tick(
    std::chrono::milliseconds duration)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  m_current_date += duration;
  m_scheduler.update_interval(duration);
}
