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

#include <iscool/schedule/manual_scheduler.hpp>
#include <iscool/schedule/setup.hpp>
#include <iscool/time/setup.hpp>

namespace bim::server::tests
{
  class fake_scheduler
  {
  public:
    fake_scheduler();

    void tick(std::chrono::milliseconds duration);

  private:
    std::chrono::nanoseconds m_current_date;
    iscool::time::scoped_time_source_delegate m_time_source_initializer;
    iscool::schedule::manual_scheduler m_scheduler;
    iscool::schedule::scoped_scheduler_delegate m_scheduler_initializer;
  };

}
