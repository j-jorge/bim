#pragma once

#include <bim/net/exchange/authentication_exchange.hpp>

#include <iscool/net/message_stream.hpp>
#include <iscool/net/socket_stream.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/shared_connection_set.hpp>

#include <optional>
#include <string>

namespace bim::net
{
  class session_handler
  {
    DECLARE_VOID_SIGNAL(connected, m_connected)
    DECLARE_SIGNAL(void(authentication_error_code), authentication_error,
                   m_authentication_error)
    DECLARE_VOID_SIGNAL(config_error, m_config_error)

  public:
    session_handler();
    ~session_handler();

    void connect(const std::string& config_url);

    const iscool::net::message_stream& message_stream() const;

    bool connected() const;
    iscool::net::session_id session_id() const;

  private:
    void connect_to_game_server(const std::vector<char>& response);
    void dispatch_error(const std::vector<char>& response) const;

  private:
    iscool::net::socket_stream m_socket_stream;
    iscool::net::message_stream m_message_stream;
    bim::net::authentication_exchange m_authentication;

    std::optional<iscool::net::session_id> m_session_id;

    iscool::signals::scoped_connection m_authentication_connection;
    iscool::signals::scoped_connection m_authentication_error_connection;
    iscool::signals::shared_connection_set m_config_request_connections;
  };
}
