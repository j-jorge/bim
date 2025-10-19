// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  class contest_result;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  struct config;

  class player_progress_tracker
  {
  public:
    player_progress_tracker(
        iscool::preferences::local_preferences& local_preferences,
        const config& config);

    ~player_progress_tracker();

    void game_over_in_public_arena(const bim::game::contest_result& result,
                                   std::uint8_t local_player_index);

  private:
    iscool::preferences::local_preferences& m_preferences;
    const config& m_config;
  };
}
