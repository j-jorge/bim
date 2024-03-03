#pragma once

#include <bim/net/exchange/authentication_exchange.hpp>

#include <iscool/net/message_stream.hpp>
#include <iscool/net/socket_stream.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <optional>
#include <string>

namespace bim::net
{
  class session_handler
  {
    DECLARE_VOID_SIGNAL(connected, m_connected)
    DECLARE_SIGNAL(void(authentication_error_code), authentication_error,
                   m_authentication_error)

  public:
    session_handler();
    ~session_handler();

    void connect(const std::string& host);

    const iscool::net::message_stream& message_stream() const;

    bool connected() const;
    iscool::net::session_id session_id() const;

  private:
    iscool::net::socket_stream m_socket_stream;
    iscool::net::message_stream m_message_stream;
    bim::net::authentication_exchange m_authentication;

    std::optional<iscool::net::session_id> m_session_id;

    iscool::signals::scoped_connection m_authentication_connection;
    iscool::signals::scoped_connection m_authentication_error_connection;
  };
}
