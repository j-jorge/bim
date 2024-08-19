// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest_runner.hpp>

#include <bim/game/contest.hpp>

bim::game::contest_runner::contest_runner(contest& contest)
  : m_contest(contest)
{}

bim::game::contest_result
bim::game::contest_runner::run(std::chrono::nanoseconds elapsed_wall_time)
{
  for (int i = 0, n = m_tick_counter.add(elapsed_wall_time,
                                         bim::game::contest::tick_interval);
       i != n; ++i)
    {
      const contest_result result = m_contest.tick();

      if (!result.still_running())
        return result;
    }

  return contest_result::create_still_running();
}
