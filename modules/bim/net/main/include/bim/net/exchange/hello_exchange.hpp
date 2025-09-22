// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/hello_ok.hpp>

#include <iscool/net/message/message.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::net
{
  class hello_exchange
  {
    DECLARE_SIGNAL(void(const hello_ok&), updated, m_updated)

  public:
    explicit hello_exchange(const iscool::net::message_stream& stream);
    ~hello_exchange();

    void start();
    void stop();

  private:
    void refresh();

    void send_message();
    void interpret_received_message(const iscool::net::message& message);
    void check_ok(const iscool::net::message& m);

  private:
    iscool::net::message_channel m_message_channel;
    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::schedule::scoped_connection m_send_message_connection;
    iscool::schedule::scoped_connection m_refresh_connection;

    std::chrono::seconds m_last_update;

    client_token m_token;
    iscool::net::message m_client_message;
  };
}
