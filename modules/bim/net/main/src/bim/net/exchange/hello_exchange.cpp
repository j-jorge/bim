// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/exchange/hello_exchange.hpp>

#include <bim/net/message/hello.hpp>
#include <bim/net/message/hello_ok.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/random/rand.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/time/now.hpp>

static constexpr std::chrono::seconds g_hello_ok_refresh_interval(30);

IMPLEMENT_SIGNAL(bim::net::hello_exchange, updated, m_updated);

bim::net::hello_exchange::hello_exchange(
    const iscool::net::message_stream& stream)
  : m_message_channel(stream, 0, 0)
  , m_last_update(0)
{}

bim::net::hello_exchange::~hello_exchange() = default;

void bim::net::hello_exchange::start()
{
  if (m_send_message_connection.connected())
    return;

  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  if ((m_last_update.count() == 0)
      || (now - m_last_update >= g_hello_ok_refresh_interval))
    refresh();
  else
    m_refresh_connection = iscool::schedule::delayed_call(
        [this]()
        {
          refresh();
        },
        g_hello_ok_refresh_interval - (now - m_last_update));
}

void bim::net::hello_exchange::stop()
{
  m_channel_signal_connection.disconnect();
  m_send_message_connection.disconnect();
  m_refresh_connection.disconnect();
}

void bim::net::hello_exchange::refresh()
{
  m_token = iscool::random::rand::get_default().random<client_token>();
  hello(m_token).build_message(m_client_message);

  m_channel_signal_connection = m_message_channel.connect_to_message(
      std::bind(&hello_exchange::interpret_received_message, this,
                std::placeholders::_2));

  send_message();
}

void bim::net::hello_exchange::send_message()
{
  m_send_message_connection = iscool::schedule::delayed_call(
      [this]()
      {
        send_message();
      },
      std::chrono::seconds(1));
  m_message_channel.send(m_client_message);
}

void bim::net::hello_exchange::interpret_received_message(
    const iscool::net::message& message)
{
  if (message.get_type() == message_type::hello_ok)
    check_ok(message);
}

void bim::net::hello_exchange::check_ok(const iscool::net::message& m)
{
  const std::optional<hello_ok> message = try_deserialize_message<hello_ok>(m);

  if (!message)
    return;

  if (message->request_token != m_token)
    return;

  m_send_message_connection.disconnect();

  m_last_update = iscool::time::now<std::chrono::seconds>();

  m_refresh_connection = iscool::schedule::delayed_call(
      [this]()
      {
        refresh();
      },
      g_hello_ok_refresh_interval);

  m_updated(*message);
}
