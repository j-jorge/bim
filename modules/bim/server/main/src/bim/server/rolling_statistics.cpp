// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/rolling_statistics.hpp>

#include <cassert>

bim::server::rolling_statistics::rolling_statistics(
    std::chrono::nanoseconds bucket_duration,
    std::chrono::nanoseconds window_duration)
  : m_total(0)
  , m_bucket_duration(bucket_duration)
  , m_window_duration(window_duration)
{
  m_dates.reserve(window_duration.count() / bucket_duration.count() + 1);
  m_values.reserve(m_dates.capacity());
}

bim::server::rolling_statistics::~rolling_statistics() = default;

std::uint32_t bim::server::rolling_statistics::total() const
{
  return m_total;
}

void bim::server::rolling_statistics::push(std::chrono::nanoseconds now,
                                           std::uint32_t value)
{
  if (!m_dates.empty() && (now < m_dates.back()))
    return;

  std::size_t i = 0;

  for (std::size_t n = m_dates.size();
       (i != n) && (now - m_dates[i] > m_window_duration); ++i)
    ;

  for (std::size_t j = 0; j != i; ++j)
    {
      assert(m_values[j] <= m_total);
      m_total -= m_values[j];
    }

  m_values.erase(m_values.begin(), m_values.begin() + i);
  m_dates.erase(m_dates.begin(), m_dates.begin() + i);

  if (!m_dates.empty() && (now - m_dates.back() < m_bucket_duration))
    m_values.back() += value;
  else
    {
      m_dates.push_back(now);
      m_values.push_back(value);
    }

  m_total += value;
}
