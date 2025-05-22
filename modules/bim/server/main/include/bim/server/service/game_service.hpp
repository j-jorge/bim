// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/feature_flags_fwd.hpp>
#include <bim/server/service/server_stats.hpp>

#include <iscool/net/message_pool.hpp>
#include <iscool/net/message_stream.hpp>

#include <iscool/signals/scoped_connection.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <optional>
#include <random>

namespace bim::net
{
  class game_update_from_client;
}

namespace bim::server
{
  class contest_timeline_service;

  struct config;
  struct game_info;

  class game_service
  {
  public:
    game_service(const config& config, iscool::net::socket_stream& socket,
                 server_stats& stats);
    ~game_service();

    bool is_in_active_game(iscool::net::session_id session) const;
    bool is_playing(iscool::net::channel_id channel) const;

    std::optional<game_info> find_game(iscool::net::channel_id channel) const;

    game_info
    new_game(std::uint8_t player_count, bim::game::feature_flags features,
             const std::array<iscool::net::session_id,
                              bim::game::g_max_player_count>& sessions);

    void process(const iscool::net::endpoint& endpoint,
                 const iscool::net::message& message);

  private:
    server_stats& m_server_stats;
    struct game;
    using game_map = boost::unordered_map<iscool::net::channel_id, game>;
    using session_to_channel_map =
        boost::unordered_map<iscool::net::session_id, iscool::net::channel_id>;

  private:
    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       iscool::net::channel_id channel, game& game,
                       std::chrono::nanoseconds now);
    void push_update(const iscool::net::endpoint& endpoint,
                     iscool::net::channel_id channel,
                     const iscool::net::message& message, game& game,
                     std::chrono::nanoseconds now);

    std::optional<std::size_t>
    validate_message(const bim::net::game_update_from_client& message,
                     iscool::net::session_id session, std::size_t player_index,
                     const game& game) const;
    void queue_actions(const bim::net::game_update_from_client& message,
                       std::size_t player_index, game& game);
    void send_actions(const iscool::net::endpoint& endpoint,
                      iscool::net::session_id session,
                      iscool::net::channel_id channel,
                      std::size_t player_index, const game& game);
    void send_game_over(const iscool::net::endpoint& endpoint,
                        iscool::net::session_id session,
                        iscool::net::channel_id channel, const game& game);

    void check_drop_desynchronized_player(iscool::net::channel_id channel,
                                          game& game,
                                          std::chrono::nanoseconds now) const;

    void schedule_clean_up();
    void clean_up();
    void clean_up(iscool::net::channel_id channel, const game& g);

  private:
    iscool::net::message_stream m_message_stream;
    iscool::net::channel_id m_next_game_channel;
    game_map m_games;
    session_to_channel_map m_session_to_channel;
    std::mt19937_64 m_random;

    iscool::signals::scoped_connection m_clean_up_connection;
    std::chrono::seconds m_clean_up_interval;

    std::unique_ptr<contest_timeline_service> m_contest_timeline_service;
    const int m_disconnection_lateness_threshold_in_ticks;
    const int m_disconnection_earliness_threshold_in_ticks;
    const std::chrono::seconds m_disconnection_inactivity_delay;

    iscool::net::message_pool m_message_pool;
  };
}
