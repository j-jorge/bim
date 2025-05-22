// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <string>

namespace bim::server
{
  struct config
  {
    config();

    unsigned short port;

    /**
     * The seed to initialize all random generators.
     */
    std::uint64_t random_seed;

    /**
     * Time interval at which we remove the sessions from the authentication.
     */
    std::chrono::seconds authentication_clean_up_interval;

    /**
     * Time interval at which we remove the encounters from the matchmaking.
     */
    std::chrono::seconds matchmaking_clean_up_interval;

    /**
     * How long to wait for the players to be ready before automatically
     * launching a random game.
     */
    std::chrono::seconds random_game_auto_start_delay;

    /**
     * Time interval at which we remove the games for which no activity has
     * been observed.
     */
    std::chrono::seconds game_service_clean_up_interval;

    /**
     * How many ticks behind the second slowest client can the slowest client
     * be before being disconnected.
     */
    int game_service_disconnection_lateness_threshold_in_ticks;

    /**
     * How many ticks ahead of the second fastest client can the fastest client
     * be before being disconnected.
     */
    int game_service_disconnection_earliness_threshold_in_ticks;

    /**
     * How many seconds of inactivity (i.e. no message from the client) do we
     * tolerate before disconnecting a client.
     */
    std::chrono::seconds game_service_disconnection_inactivity_delay;

    /** Path to the folder where to store the contest timelines. */
    std::string contest_timeline_folder;

    /**
     * Tells if we record the games played in this server. The games are saved
     * in contest_timeline_folder.
     */
    bool enable_contest_timeline_recording;

    /**
     * How long we wait after a stat is changed to record it in the logs
     */
    std::chrono::minutes stats_dump_delay;

    /**
     * How long we wait after a stat is changed to record it in the logs
     */
    std::chrono::days log_rotation_interval;
  };
}
