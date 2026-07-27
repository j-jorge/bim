// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/hello_ok.hpp>

#include <iscool/net/message_pool.hpp>
#include <iscool/net/message_stream.hpp>
#include <iscool/schedule/scoped_connection.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <span>

namespace bim::net
{
  class authentication;
}

namespace bim::server
{
  struct config;
  struct create_session_result;
  class session_service;
  class statistics_service;

  class authentication_service
  {
    DECLARE_SIGNAL(void(const iscool::net::endpoint&,
                        const iscool::net::message& message),
                   message, m_message)

  public:
    authentication_service(const config& config,
                           iscool::net::socket_stream& socket,
                           session_service& sessions,
                           statistics_service& statistics);
    ~authentication_service();

  private:
    void check_session(const iscool::net::endpoint& endpoint,
                       const iscool::net::message& message);

    void check_authentication(const iscool::net::endpoint& endpoint,
                              const iscool::net::message& m);
    void send_bad_protocol(const iscool::net::endpoint& endpoint,
                           const std::string& client_ip_address,
                           const bim::net::authentication& message);
    void send_accepted(const iscool::net::endpoint& endpoint,
                       bim::net::client_token token,
                       iscool::net::session_id session);

    void send_refused(const iscool::net::endpoint& endpoint,
                      const std::string& client_ip_address,
                      bim::net::client_token token);

    void check_hello(const iscool::net::endpoint& endpoint,
                     const iscool::net::message& m);

    void send_acknowledge_keep_alive(const iscool::net::endpoint& endpoint,
                                     iscool::net::session_id session);

    void sessions_ready(std::span<const create_session_result> results);

    void schedule_clean_up();
    void clean_up();

  private:
    struct pending_authentication;

    using pending_authentication_map =
        boost::unordered_map<bim::net::client_token, pending_authentication>;

  private:
    session_service& m_session_service;
    statistics_service& m_statistics;

    const iscool::net::socket_stream& m_socket;
    iscool::net::message_stream m_message_stream;

    iscool::net::message_pool m_message_pool;

    iscool::signals::scoped_connection m_session_connection;
    pending_authentication_map m_pending_authentication;

    bim::net::hello_ok m_hello_ok;

    iscool::schedule::scoped_connection m_clean_up_connection;
    const std::chrono::seconds m_clean_up_interval;
    const std::chrono::seconds m_pending_authentication_removal_delay;
  };
}
