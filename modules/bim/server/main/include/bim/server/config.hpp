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
     * Time interval at which we check and remove inactive sessions.
     */
    std::chrono::seconds session_clean_up_interval;

    /**
     * Inactivity delay after which a session becomes eligible for removal.
     */
    std::chrono::seconds session_removal_delay;

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

    /**
     * The duration under which a game is considered short.
     */
    std::chrono::seconds game_service_max_duration_for_short_game;

    /**
     * Coins reward for the winner of a game in random matchmaking.
     */
    std::uint16_t game_service_coins_per_victory;

    /**
     * Coins reward for the losers of a game in random matchmaking.
     */
    std::uint16_t game_service_coins_per_defeat;

    /**
     * Coins reward for all players of a draw game in random matchmaking.
     */
    std::uint16_t game_service_coins_per_draw;

    /**
     * Coins reward for the winner of a game in random matchmaking when the
     * game was short.
     */
    std::uint16_t game_service_coins_per_short_game_victory;

    /**
     * Coins reward for the loser of a game in random matchmaking when the game
     * was short..
     */
    std::uint16_t game_service_coins_per_short_game_defeat;

    /**
     * Coins reward for all players of a draw a game in random matchmaking when
     * the game was short.
     */
    std::uint16_t game_service_coins_per_short_game_draw;

    /** Path to the folder where to store the contest timelines. */
    std::string contest_timeline_folder;

    /**
     * Tells if we record the games played in this server. The games are saved
     * in contest_timeline_folder.
     */
    bool enable_contest_timeline_recording;

    /**
     * How many seconds after the last request for a given IP to be removed
     * from the geolocation service. The IP will receive a new ID on the next
     * request.
     */
    std::chrono::seconds geolocation_clean_up_interval;

    /**
     * Interval at which we reopen the GeoIP database, to get fresh data.
     */
    std::chrono::minutes geolocation_update_interval;

    /**
     * The path to the GeoIP database.
     */
    std::string geolocation_database_path;

    /**
     * Whether or not we use IP geolocation.
     */
    bool enable_geolocation;

    /** Tells if we log the statistics. */
    bool enable_statistics_log;

    /** Whether or not we should enable the statistics on a rolling window. */
    bool enable_rolling_statistics;

    /** How long we wait after a stat is changed to record it in the logs. */
    std::chrono::seconds statistics_dump_delay;

    /** Path to the folder where to store the server stats. */
    std::string statistics_log_file;

    /** The name of the server, as sent to the clients. */
    std::string name;

    /**
     * Whether or not we send notifications to a Discord channel when players
     * are waiting for a random opponent.
     */
    bool enable_discord_matchmaking_notifications;

    /**
     * Discord endpoint where to send a message when a player is waiting for a
     * random opponent.
     */
    std::string discord_matchmaking_notification_url;

    /**
     * How long we wait between two messages sent to
     * discord_matchmaking_notification_url.
     */
    std::chrono::seconds discord_matchmaking_notification_interval;

    /**
     * Whether or not we should enable the karma service to filter incoming
     * connections.
     */
    bool enable_karma;

    /** How long IPs with negative karma are blacklisted. */
    std::chrono::minutes karma_blacklisting_duration;

    /** The interval between two reviews of the blacklisted IPs. */
    std::chrono::minutes karma_review_interval;

    /** Karma value assigned to new connections. */
    std::int8_t initial_karma_value;

    /** Value added to the karma when a player is disconnected. */
    std::int8_t disconnection_karma_adjustment;

    /** Value added to the karma when a player dies in a short game. */
    std::int8_t short_game_karma_adjustment;

    /** Value added to the karma when a player behaves correctly. */
    std::int8_t good_behavior_karma_adjustment;
  };
}
