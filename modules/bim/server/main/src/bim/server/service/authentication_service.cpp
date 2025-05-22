// SPDX-License-Identifier: AGPL-3.0-only
#include "bim/server/service/server_stats.hpp"
#include <bim/server/service/authentication_service.hpp>

#include <bim/server/config.hpp>

#include <bim/net/message/acknowledge_keep_alive.hpp>
#include <bim/net/message/authentication.hpp>
#include <bim/net/message/authentication_ko.hpp>
#include <bim/net/message/authentication_ok.hpp>
#include <bim/net/message/keep_alive.hpp>
#include <bim/net/message/protocol_version.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
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
  bim::net::client_token token;
  std::chrono::nanoseconds release_at_this_date;
};

bim::server::authentication_service::authentication_service(
    const config& config, iscool::net::socket_stream& socket,
    server_stats& stats)
  : m_server_stats(stats)
  , m_message_stream(socket)
  , m_next_session_id(1)
  , m_clean_up_interval(config.authentication_clean_up_interval)
  , m_message_pool(64)
{
  m_message_stream.connect_to_message(
      std::bind(&authentication_service::check_session, this,
                std::placeholders::_1, std::placeholders::_2));

  schedule_clean_up();
}

bim::server::authentication_service::~authentication_service() = default;

void bim::server::authentication_service::check_session(
    const iscool::net::endpoint& endpoint, const iscool::net::message& message)
{
  const iscool::net::session_id session = message.get_session_id();

  if (session == 0)
    {
      if (message.get_type() == bim::net::message_type::authentication)
        check_authentication(endpoint, message);

      return;
    }

  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  it->second.release_at_this_date = authentication_date_for_next_release();

  if (message.get_type() == bim::net::message_type::keep_alive)
    {
      send_acknowledge_keep_alive(endpoint, session);
      return;
    }

  m_message(endpoint, message);
}

void bim::server::authentication_service::check_authentication(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)
{
  const std::optional<bim::net::authentication> message =
      bim::net::try_deserialize_message<bim::net::authentication>(m);

  const bim::net::client_token token = message->get_request_token();

  ic_log(iscool::log::nature::info(), "authentication_service",
         "Received authentication request from token {}.", token);

  if (message->get_protocol_version() != bim::net::protocol_version)
    {
      ic_log(iscool::log::nature::info(), "server",
             "Authentication request from token {}, ip={}: bad protocol {}.",
             token, endpoint.address().to_string(),
             message->get_protocol_version());

      const iscool::net::message_pool::slot s =
          m_message_pool.pick_available();
      bim::net::authentication_ko(
          token, bim::net::authentication_error_code::bad_protocol)
          .build_message(*s.value);

      m_message_stream.send(endpoint, *s.value);
      m_message_pool.release(s.id);

      return;
    }

  const iscool::net::session_id session = m_next_session_id;

  session_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_sessions.emplace(token, session);

  ic_log(iscool::log::nature::info(), "server",
         "Attach session {} to token {} from ip={}.", it->second, token,
         endpoint.address().to_string());

  if (inserted)
    {
      ++m_next_session_id;

      client_info client{ .token = token,
                          .release_at_this_date =
                              authentication_date_for_next_release() };

      m_clients.emplace(session, std::move(client));
      m_server_stats.record_session_connected();
    }

  const iscool::net::message_pool::slot s = m_message_pool.pick_available();
  bim::net::authentication_ok(token, it->second).build_message(*s.value);

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

  for (client_map::iterator it = m_clients.begin(), eit = m_clients.end();
       it != eit;)
    if (it->second.release_at_this_date <= now)
      {
        ic_log(iscool::log::nature::info(), "server", "Disconnected {}.",
               it->first);
        m_sessions.erase(it->second.token);
        it = m_clients.erase(it);
        m_server_stats.record_session_disconnected();
      }
    else
      ++it;
}
