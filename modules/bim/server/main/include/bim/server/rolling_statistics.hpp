// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <cstdint>

namespace bim::server
{
  class rolling_statistics
  {
  public:
    rolling_statistics(std::chrono::nanoseconds bucket_duration,
                       std::chrono::nanoseconds window_duration);
    ~rolling_statistics();

    std::uint32_t total() const;

    void push(std::chrono::nanoseconds now, std::uint32_t value);

  private:
    std::vector<std::chrono::nanoseconds> m_dates;
    std::vector<std::uint32_t> m_values;
    std::uint32_t m_total;

    std::chrono::nanoseconds m_bucket_duration;
    std::chrono::nanoseconds m_window_duration;
  };
}
