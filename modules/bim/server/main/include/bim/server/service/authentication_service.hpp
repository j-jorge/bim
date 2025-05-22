// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include "bim/server/service/server_stats.hpp"
#include <bim/net/message/client_token.hpp>

#include <iscool/net/message_pool.hpp>
#include <iscool/net/message_stream.hpp>

#include <boost/unordered/unordered_map.hpp>

namespace bim::net
{
  class authentication;
}

namespace bim::server
{
  struct config;

  class authentication_service
  {
    DECLARE_SIGNAL(void(const iscool::net::endpoint&,
                        const iscool::net::message& message),
                   message, m_message)

  public:
    authentication_service(const config& config,
                           iscool::net::socket_stream& socket,
                           server_stats& stats);
    ~authentication_service();

  private:
    server_stats& m_server_stats;
    using session_map =
        boost::unordered_map<bim::net::client_token, iscool::net::session_id>;

    struct client_info;

    using client_map =
        boost::unordered_map<iscool::net::session_id, client_info>;

  private:
    void check_session(const iscool::net::endpoint& endpoint,
                       const iscool::net::message& message);

    void check_authentication(const iscool::net::endpoint& endpoint,
                              const iscool::net::message& m);

    void send_acknowledge_keep_alive(const iscool::net::endpoint& endpoint,
                                     iscool::net::session_id session);

    void schedule_clean_up();

    void clean_up();

  private:
    iscool::net::message_stream m_message_stream;
    iscool::net::session_id m_next_session_id;

    session_map m_sessions;
    client_map m_clients;

    iscool::signals::scoped_connection m_clean_up_connection;
    std::chrono::seconds m_clean_up_interval;

    iscool::net::message_pool m_message_pool;
  };
}
