// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/authentication_service.hpp>

#include <bim/server/service/session_service.hpp>
#include <bim/server/service/statistics_service.hpp>

#include <bim/net/message/acknowledge_keep_alive.hpp>
#include <bim/net/message/authentication.hpp>
#include <bim/net/message/authentication_ko.hpp>
#include <bim/net/message/authentication_ok.hpp>
#include <bim/net/message/hello.hpp>
#include <bim/net/message/hello_ok.hpp>
#include <bim/net/message/keep_alive.hpp>
#include <bim/net/message/protocol_version.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <bim/tracy.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/net/socket_stream.hpp>
#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::server::authentication_service, message, m_message);

bim::server::authentication_service::authentication_service(
    const config& config, iscool::net::socket_stream& socket,
    session_service& sessions, statistics_service& statistics)
  : m_session_service(sessions)
  , m_statistics(statistics)
  , m_socket(socket)
  , m_message_stream(socket)
  , m_message_pool(64)
{
  m_hello_ok.version = bim::net::protocol_version;
  m_hello_ok.name = config.name;

  m_message_stream.connect_to_message(
      std::bind(&authentication_service::check_session, this,
                std::placeholders::_1, std::placeholders::_2));

#if BIM_ENABLE_TRACY
  TracyPlotConfig("Bytes in", tracy::PlotFormatType::Memory, true, false, 0);
  TracyPlotConfig("Bytes out", tracy::PlotFormatType::Memory, true, false, 0);

  socket.connect_to_received(
      [this](const iscool::net::endpoint&, const iscool::net::byte_array&)
      {
        TracyPlot("Bytes in", (std::int64_t)m_socket.received_bytes());
        TracyPlot("Bytes out", (std::int64_t)m_socket.sent_bytes());
      });
#endif
}

bim::server::authentication_service::~authentication_service() = default;

void bim::server::authentication_service::check_session(
    const iscool::net::endpoint& endpoint, const iscool::net::message& message)
{
  const iscool::net::session_id session = message.get_session_id();

  if (session == 0)
    {
      switch (message.get_type())
        {
        case bim::net::message_type::authentication:
          check_authentication(endpoint, message);
          break;
        case bim::net::message_type::hello:
          check_hello(endpoint, message);
          break;
        }
    }
  else if (m_session_service.refresh_session(session))
    {
      if (message.get_type() == bim::net::message_type::keep_alive)
        send_acknowledge_keep_alive(endpoint, session);
      else
        m_message(endpoint, message);
    }

  m_statistics.network_traffic(m_socket.received_bytes(),
                               m_socket.sent_bytes());
}

void bim::server::authentication_service::check_authentication(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)
{
  const std::optional<bim::net::authentication> message =
      bim::net::try_deserialize_message<bim::net::authentication>(m);

  if (!message)
    return;

  const bim::net::client_token token = message->get_request_token();

  ic_log(iscool::log::nature::info(), "authentication_service",
         "Received authentication request from token {}.", token);

  const std::string client_ip_address = endpoint.address().to_string();

  if (message->get_protocol_version() != bim::net::protocol_version)
    {
      send_bad_protocol(endpoint, client_ip_address, *message);
      return;
    }

  const std::optional<iscool::net::session_id> session =
      m_session_service.create_or_refresh_session(endpoint.address(), token);

  if (!session)
    {
      send_refused(endpoint, client_ip_address, *message);
      return;
    }

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::authentication_ok(token, *session).build_message(*s.value);

  m_message_stream.send(endpoint, *s.value);
  m_message_pool.release(s.id);
}

void bim::server::authentication_service::send_bad_protocol(
    const iscool::net::endpoint& endpoint,
    const std::string& client_ip_address,
    const bim::net::authentication& message)
{
  const bim::net::client_token token = message.get_request_token();

  ic_log(iscool::log::nature::info(), "authentication_service",
         "Authentication request from token {}: bad protocol {}.", token,
         client_ip_address, message.get_protocol_version());

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::authentication_ko(
      token, bim::net::authentication_error_code::bad_protocol)
      .build_message(*s.value);

  m_message_stream.send(endpoint, *s.value);
  m_message_pool.release(s.id);
}

void bim::server::authentication_service::send_refused(
    const iscool::net::endpoint& endpoint,
    const std::string& client_ip_address,
    const bim::net::authentication& message)
{
  ic_log(iscool::log::nature::info(), "authentication_service",
         "Authentication request from blacklisted IP {}.", client_ip_address);

  const bim::net::client_token token = message.get_request_token();

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::authentication_ko(token,
                              bim::net::authentication_error_code::refused)
      .build_message(*s.value);

  m_message_stream.send(endpoint, *s.value);
  m_message_pool.release(s.id);
}

void bim::server::authentication_service::check_hello(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)
{
  const std::optional<bim::net::hello> message =
      bim::net::try_deserialize_message<bim::net::hello>(m);

  if (!message)
    return;

  const bim::net::client_token token = message->get_request_token();

  ic_log(iscool::log::nature::info(), "authentication_service",
         "Received hello from token {}.", token);

  m_hello_ok.request_token = token;

  m_hello_ok.games_now = m_statistics.games_now();
  m_hello_ok.games_last_hour = m_statistics.games_last_hour();
  m_hello_ok.games_last_day = m_statistics.games_last_day();
  m_hello_ok.games_last_month = m_statistics.games_last_month();

  m_hello_ok.sessions_now = m_statistics.sessions_now();
  m_hello_ok.sessions_last_hour = m_statistics.sessions_last_hour();
  m_hello_ok.sessions_last_day = m_statistics.sessions_last_day();
  m_hello_ok.sessions_last_month = m_statistics.sessions_last_month();

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  m_hello_ok.build_message(*s.value);

  m_message_stream.send(endpoint, *s.value);
  m_message_pool.release(s.id);
}

void bim::server::authentication_service::send_acknowledge_keep_alive(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session)
{
  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::acknowledge_keep_alive().build_message(*s.value);

  m_message_stream.send(endpoint, *s.value, session, 0);
  m_message_pool.release(s.id);
}
