// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace bim::axmol::schedule
{
  class bridge
  {
  public:
    bridge();
    ~bridge();

    bridge(const bridge&) = delete;
    bridge& operator=(const bridge&) = delete;

  private:
    void delayed_call(const std::function<void()>& f,
                      const std::chrono::nanoseconds& delay);

    void schedule_call(const std::function<void()>& f,
                       const std::chrono::nanoseconds& delay);

  private:
    std::size_t m_next_call_id;
    const std::string m_key_prefix;
    std::string m_call_key_buffer;
  };
}
