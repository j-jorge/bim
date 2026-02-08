// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/geolocation_service.hpp>
#include <bim/server/service/karma_service.hpp>

#include <bim/net/message/client_token.hpp>

#include <iscool/net/message/session_id.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <boost/asio/ip/address.hpp>
#include <boost/unordered/unordered_map.hpp>

#include <optional>

namespace bim::server
{
  struct config;
  class karma_service;
  class statistics_service;

  class session_service
  {
  public:
    session_service(const config& config, statistics_service& statistics);
    ~session_service();

    std::optional<iscool::net::session_id>
    create_or_refresh_session(const boost::asio::ip::address& address,
                              bim::net::client_token token);
    bool refresh_session(iscool::net::session_id session);

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

  private:
    geolocation_service m_geoloc;
    karma_service m_karma;
    statistics_service& m_statistics;

    iscool::net::session_id m_next_session_id;

    session_map m_sessions;
    client_map m_clients;

    iscool::schedule::scoped_connection m_clean_up_connection;
    const std::chrono::seconds m_clean_up_interval;
    const std::chrono::seconds m_session_removal_delay;
  };
}
