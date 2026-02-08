// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/hello_ok.hpp>

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
    void send_refused(const iscool::net::endpoint& endpoint,
                      const std::string& client_ip_address,
                      const bim::net::authentication& message);

    void check_hello(const iscool::net::endpoint& endpoint,
                     const iscool::net::message& m);

    void send_acknowledge_keep_alive(const iscool::net::endpoint& endpoint,
                                     iscool::net::session_id session);

  private:
    session_service& m_session_service;
    statistics_service& m_statistics;

    const iscool::net::socket_stream& m_socket;
    iscool::net::message_stream m_message_stream;

    iscool::net::message_pool m_message_pool;

    bim::net::hello_ok m_hello_ok;
  };
}
