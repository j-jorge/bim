// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/authentication_service.hpp>

#include <bim/server/config.hpp>
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
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>
#include <iscool/time/now.hpp>

IMPLEMENT_SIGNAL(bim::server::authentication_service, message, m_message);

static std::chrono::nanoseconds authentication_date_for_next_release()
{
  return iscool::time::now<std::chrono::nanoseconds>()
         + std::chrono::minutes(10);
}

struct bim::server::authentication_service::client_info
{
  boost::asio::ip::address address;
  bim::net::client_token token;
  std::chrono::nanoseconds release_at_this_date;
};

bim::server::authentication_service::authentication_service(
    const config& config, iscool::net::socket_stream& socket,
    statistics_service& statistics)
  : m_geoloc(config)
  , m_karma(config)
  , m_statistics(statistics)
  , m_socket(socket)
  , m_message_stream(socket)
  , m_next_session_id(1)
  , m_clean_up_interval(config.authentication_clean_up_interval)
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

  schedule_clean_up();
}

bim::server::authentication_service::~authentication_service() = default;

void bim::server::authentication_service::update_karma_disconnection(
    iscool::net::session_id session)
{
  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  ic_log(iscool::log::nature::info(), "authentication_service",
         "Internal disconnection for session={}.", session);

  m_karma.disconnection(it->second.address);
  disconnect(it);
}

void bim::server::authentication_service::update_karma_short_game(
    iscool::net::session_id session)
{
  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  if (m_karma.short_game(it->second.address)
      == karma_service::update_result::kick_out)
    disconnect(it);
}

void bim::server::authentication_service::update_karma_good_behavior(
    iscool::net::session_id session)
{
  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  m_karma.good_behavior(it->second.address);
}

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
  else
    {
      const client_map::iterator it = m_clients.find(session);

      if (it != m_clients.end())
        {
          it->second.release_at_this_date =
              authentication_date_for_next_release();

          if (message.get_type() == bim::net::message_type::keep_alive)
            send_acknowledge_keep_alive(endpoint, session);
          else
            m_message(endpoint, message);
        }
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

  if (!m_karma.allowed(endpoint.address()))
    {
      send_refused(endpoint, client_ip_address, *message);
      return;
    }

  const iscool::net::session_id session = m_next_session_id;

  session_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_sessions.emplace(token, session);

  if (inserted)
    {
      const geolocation_service::address_info address_info =
          m_geoloc.lookup(client_ip_address);

      ic_log(iscool::log::nature::info(), "authentication_service",
             "Attach session {} to token {}, id={}, country_code={}, "
             "country='{}'.",
             it->second, token, address_info.id, address_info.country_code,
             address_info.country);

      ++m_next_session_id;

      client_info client{ .address = endpoint.address(),
                          .token = token,
                          .release_at_this_date =
                              authentication_date_for_next_release() };

      m_clients.emplace(session, std::move(client));
      m_statistics.record_session_connected();
    }

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::authentication_ok(token, it->second).build_message(*s.value);

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

void bim::server::authentication_service::disconnect(
    const client_map::iterator& it)
{
  m_sessions.erase(it->second.token);
  m_clients.erase(it);

  m_statistics.record_session_disconnected(1);
}

void bim::server::authentication_service::schedule_clean_up()
{
  m_clean_up_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        clean_up();
        schedule_clean_up();
      },
      m_clean_up_interval);
}

void bim::server::authentication_service::clean_up()
{
  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();
  std::uint64_t disconnected_count = 0;

  for (client_map::iterator it = m_clients.begin(), eit = m_clients.end();
       it != eit;)
    if (it->second.release_at_this_date <= now)
      {
        ic_log(iscool::log::nature::info(), "authentication_service",
               "Disconnected {}.", it->first);
        m_sessions.erase(it->second.token);
        it = m_clients.erase(it);
        ++disconnected_count;
      }
    else
      ++it;

  m_statistics.record_session_disconnected(disconnected_count);
}
