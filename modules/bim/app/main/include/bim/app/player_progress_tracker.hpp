// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::net
{
  struct contest_result;
}

namespace bim::app
{
  class analytics_service;

  class player_progress_tracker
  {
  public:
    explicit player_progress_tracker(analytics_service& analytics);

    void game_over_in_public_arena(const bim::net::contest_result& result,
                                   std::uint8_t local_player_index);

  private:
    analytics_service& m_analytics;
  };
}
