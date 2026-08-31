// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/scheduler.hpp>

bim::app::tests::scheduler::scheduler()
  : m_current_date{}
  , m_time_source_initializer(
        [this]() -> std::chrono::nanoseconds
          {
            return m_current_date;
          },
        [this]() -> std::chrono::nanoseconds
          {
            return m_current_date;
          })
  , m_scheduler_initializer(m_scheduler.get_delayed_call_delegate())
{}

bim::app::tests::scheduler::~scheduler() = default;

void bim::app::tests::scheduler::tick(std::chrono::nanoseconds d)
{
  m_current_date += d;
  m_scheduler.update_interval(d);
}
