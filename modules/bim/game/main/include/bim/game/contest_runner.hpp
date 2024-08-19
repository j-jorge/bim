// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_result.hpp>
#include <bim/game/tick_counter.hpp>

namespace bim::game
{
  class contest;

  class contest_runner
  {
  public:
    explicit contest_runner(contest& contest);

    contest_result run(std::chrono::nanoseconds elapsed_wall_time);

  private:
    contest& m_contest;
    tick_counter m_tick_counter;
  };
}
