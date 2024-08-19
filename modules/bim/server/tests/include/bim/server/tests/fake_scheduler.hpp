// SPDX-License-Identifier: AGPL-3.0-only
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
