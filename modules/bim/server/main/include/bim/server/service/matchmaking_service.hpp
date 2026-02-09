// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/game_reward_availability_fwd.hpp>

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/net/message_pool.hpp>
#include <iscool/net/message_stream.hpp>
#include <iscool/schedule/scoped_connection.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <optional>
#include <random>
#include <span>
#include <string>
#include <vector>

namespace bim::server
{
  class game_service;

  struct config;

  class matchmaking_service
  {
  public:
    enum class try_start_mode
    {
      wait,
      now
    };

    struct join_encounter_result
    {
      bim::net::encounter_id id;
      uint8_t player_count;
    };

    struct kick_session_event
    {
      iscool::net::session_id session;
      bim::net::encounter_id encounter_id;
    };

  public:
    matchmaking_service(const config& config,
                        iscool::net::socket_stream& socket,
                        game_service& game_service,
                        game_reward_availability reward_availability);
    ~matchmaking_service();

    bim::net::encounter_id new_encounter(const iscool::net::endpoint& endpoint,
                                         iscool::net::session_id session,
                                         bim::net::client_token request_token);
    bool refresh_encounter(bim::net::encounter_id encounter_id,
                           const iscool::net::endpoint& endpoint,
                           iscool::net::session_id session,
                           bim::net::client_token request_token);
    std::optional<join_encounter_result>
    add_in_any_encounter(const iscool::net::endpoint& endpoint,
                         iscool::net::session_id session,
                         bim::net::client_token request_token);

    void mark_as_ready(const iscool::net::endpoint& endpoint,
                       iscool::net::session_id session,
                       bim::net::encounter_id encounter_id,
                       bim::net::client_token request_token,
                       bim::game::feature_flags features,
                       try_start_mode try_start);

    std::span<const bim::net::encounter_id> garbage_encounters() const;
    std::span<const kick_session_event> garbage_sessions() const;

    void drop_garbage();

  private:
    struct encounter_info;
    using encounter_map =
        boost::unordered_map<bim::net::encounter_id, encounter_info>;

  private:
    void refresh_encounter(bim::net::encounter_id encounter_id,
                           encounter_info& encounter,
                           const iscool::net::endpoint& endpoint,
                           iscool::net::session_id session,
                           bim::net::client_token request_token,
                           std::size_t session_index);

    void send_game_on_hold(const iscool::net::endpoint& endpoint,
                           bim::net::client_token token,
                           iscool::net::session_id session,
                           bim::net::encounter_id encounter_id,
                           std::uint8_t player_count);

    void remove_non_ready_players(bim::net::encounter_id encounter_id,
                                  encounter_info& encounter);

    void remove_inactive_sessions(bim::net::encounter_id encounter_id,
                                  encounter_info& encounter);
    void remove_inactive_sessions(std::chrono::nanoseconds now,
                                  bim::net::encounter_id encounter_id,
                                  encounter_info& encounter);

    void schedule_clean_up();

    void clean_up();

  private:
    iscool::net::message_stream m_message_stream;
    game_service& m_game_service;
    const game_reward_availability m_reward_availability;

    encounter_map m_encounters;
    bim::net::encounter_id m_next_encounter_id;

    iscool::schedule::scoped_connection m_clean_up_connection;
    std::chrono::seconds m_clean_up_interval;

    std::vector<kick_session_event> m_done_sessions;
    std::vector<bim::net::encounter_id> m_done_encounters;

    iscool::net::message_pool m_message_pool;

    std::mt19937_64 m_random;
  };
}
