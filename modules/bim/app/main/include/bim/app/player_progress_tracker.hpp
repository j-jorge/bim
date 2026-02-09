// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::net
{
  struct contest_result;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  class analytics_service;

  class player_progress_tracker
  {
  public:
    player_progress_tracker(
        analytics_service& analytics,
        iscool::preferences::local_preferences& local_preferences);

    ~player_progress_tracker();

    void game_over_in_public_arena(const bim::net::contest_result& result,
                                   std::uint8_t local_player_index);

  private:
    analytics_service& m_analytics;
    iscool::preferences::local_preferences& m_preferences;
  };
}
