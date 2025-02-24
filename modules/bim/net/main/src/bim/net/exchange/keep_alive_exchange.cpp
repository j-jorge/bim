// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/exchange/keep_alive_exchange.hpp>

#include <bim/net/message/keep_alive.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>

static int g_max_retry_count = 10;

IMPLEMENT_SIGNAL(bim::net::keep_alive_exchange, disconnected, m_disconnected);

bim::net::keep_alive_exchange::keep_alive_exchange(
    const iscool::net::message_stream& stream)
  : m_message_channel(stream)
{
  keep_alive().build_message(m_client_message);
}

bim::net::keep_alive_exchange::~keep_alive_exchange() = default;

void bim::net::keep_alive_exchange::start(iscool::net::session_id session)
{
  m_message_channel.rebind(session, 0);

  m_retry_count = g_max_retry_count;

  m_channel_signal_connection = m_message_channel.connect_to_message(
      std::bind(&keep_alive_exchange::interpret_received_message, this,
                std::placeholders::_2));

  tick();
}

void bim::net::keep_alive_exchange::stop()
{
  m_update_connection.disconnect();
}

void bim::net::keep_alive_exchange::tick()
{
  if (m_retry_count == 0)
    {
      ic_log(iscool::log::nature::info(), "keep_alive_exchange",
             "Disconnected.");

      stop();
      m_disconnected();
      return;
    }

  m_update_connection = iscool::schedule::delayed_call(
      std::bind(&keep_alive_exchange::tick, this), std::chrono::seconds(1));

  --m_retry_count;
  m_message_channel.send(m_client_message);
}

void bim::net::keep_alive_exchange::interpret_received_message(
    const iscool::net::message& message)
{
  m_retry_count = std::min(m_retry_count + 1, g_max_retry_count);
}
