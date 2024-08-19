// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <thread>

bim::server::tests::fake_scheduler::fake_scheduler()
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

void bim::server::tests::fake_scheduler::tick(
    std::chrono::milliseconds duration)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  m_current_date += duration;
  m_scheduler.update_interval(duration);
}
