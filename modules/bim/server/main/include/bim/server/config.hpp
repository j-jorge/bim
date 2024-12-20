// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <string>

namespace bim::server
{
  struct config
  {
    unsigned short port;

    /**
     * Time interval at which we remove the encounters from the matchmaking.
     */
    std::chrono::seconds matchmaking_clean_up_interval =
        std::chrono::minutes(3);

    /**
     * Time interval at which we remove the games for which no activity has
     * been observed.
     */
    std::chrono::seconds game_service_clean_up_interval =
        std::chrono::minutes(3);

    /**
     * How many ticks behind the second slowest client can the slowest client
     * be before being disconnected.
     */
    int game_service_disconnection_lateness_threshold_in_ticks = 24;

    /**
     * How many ticks ahead of the second fastest client can the fastest client
     * be before being disconnected.
     */
    int game_service_disconnection_earliness_threshold_in_ticks = 24;

    /** Path to the folder where to store the contest timelines. */
    std::string contest_timeline_folder;

    /**
     * Tells if we record the games played in this server. The games are saved
     * in contest_timeline_folder.
     */
    bool enable_contest_timeline_recording = false;
  };
}
