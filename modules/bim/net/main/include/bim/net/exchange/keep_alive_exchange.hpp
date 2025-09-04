// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/net/message/message.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::net
{
  class keep_alive_exchange
  {
    DECLARE_VOID_SIGNAL(disconnected, m_disconnected)

  public:
    explicit keep_alive_exchange(const iscool::net::message_stream& stream);
    ~keep_alive_exchange();

    void start(iscool::net::session_id session);
    void stop();

  private:
    void tick();

    void interpret_received_message(const iscool::net::message& message);

  private:
    iscool::net::message_channel m_message_channel;

    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::schedule::scoped_connection m_update_connection;

    iscool::net::message m_client_message;

    int m_retry_count;
  };
}
