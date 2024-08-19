// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/tick_counter.hpp>

bim::game::tick_counter::tick_counter()
  : m_remainder(0)
{}

int bim::game::tick_counter::add(std::chrono::nanoseconds elapsed_time,
                                 std::chrono::nanoseconds time_step)
{
  m_remainder += elapsed_time;

  const int result = (m_remainder / time_step);
  m_remainder -= result * time_step;

  return result;
}
