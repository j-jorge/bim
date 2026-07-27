// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/create_session_result.hpp>
#include <bim/server/service/geolocation_service.hpp>
#include <bim/server/service/karma_service.hpp>

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/session_token.hpp>
#include <bim/net/message/user_id.hpp>

#include <iscool/net/message/session_id.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/shared_connection_set.hpp>

#include <boost/asio/ip/address.hpp>
#include <boost/unordered/unordered_map.hpp>

#include <json/value.h>

#include <optional>
#include <span>

namespace bim::server
{
  struct config;
  class karma_service;
  class statistics_service;

  class session_service
  {
    DECLARE_SIGNAL(void(std::span<const create_session_result>),
                   sessions_ready, m_sessions_ready)

  public:
    session_service(const config& config, statistics_service& statistics);
    ~session_service();

    iscool::net::session_id new_bot_session();

    create_session_result
    create_or_refresh_session(const boost::asio::ip::address& address,
                              bim::net::client_token token,
                              const bim::net::session_token& session_token);
    bool refresh_session(iscool::net::session_id session);

    bim::net::user_id user_id(iscool::net::session_id session) const;

    void update_karma_disconnection(iscool::net::session_id session);
    void update_karma_short_game(iscool::net::session_id session);
    void update_karma_good_behavior(iscool::net::session_id session);

  private:
    using session_map =
        boost::unordered_map<bim::net::client_token, iscool::net::session_id>;

    struct client_info;

    using client_map =
        boost::unordered_map<iscool::net::session_id, client_info>;

  private:
    std::chrono::nanoseconds date_for_next_release() const;

    void disconnect(const client_map::iterator& it);

    void schedule_clean_up();
    void clean_up();

    void schedule_user_id_request();
    void fetch_user_ids();
    void user_id_response(const Json::Value& response);
    void user_id_error(int code, std::span<const char> body);

  private:
    geolocation_service m_geoloc;
    karma_service m_karma;
    statistics_service& m_statistics;

    iscool::net::session_id m_next_real_session_id;
    iscool::net::session_id m_next_bot_session_id;

    session_map m_sessions;
    client_map m_clients;

    iscool::schedule::scoped_connection m_clean_up_connection;
    const std::chrono::seconds m_clean_up_interval;
    const std::chrono::seconds m_session_removal_delay;

    const std::string m_user_id_url;
    std::vector<std::string> m_headers;
    iscool::schedule::connection m_schedule_user_id_connection;
    iscool::signals::shared_connection_set m_user_id_connections;
    Json::Value m_user_id_business_request;
    std::vector<create_session_result> m_create_session_dispatch;
    bool m_ongoing_user_id_business_request;
  };
}
