// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>

namespace bim::game
{
  class tick_counter
  {
  public:
    tick_counter();

    int add(std::chrono::nanoseconds elapsed_time,
            std::chrono::nanoseconds time_step);

  private:
    std::chrono::nanoseconds m_remainder;
  };
}
