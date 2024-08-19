// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/authentication_service.hpp>

#include <bim/net/message/authentication.hpp>
#include <bim/net/message/authentication_ko.hpp>
#include <bim/net/message/authentication_ok.hpp>
#include <bim/net/message/protocol_version.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

IMPLEMENT_SIGNAL(bim::server::authentication_service, message, _message);

struct bim::server::authentication_service::client_info
{
  bim::net::client_token token;
  iscool::signals::scoped_connection timeout_signal_connection;
};

bim::server::authentication_service::authentication_service(
    iscool::net::socket_stream& socket)
  : m_message_stream(socket)
  , m_next_session_id(1)
{
  m_message_stream.connect_to_message(
      std::bind(&authentication_service::check_session, this,
                std::placeholders::_1, std::placeholders::_2));
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
    }
  else
    {
      const client_map::iterator it = m_clients.find(session);

      if (it != m_clients.end())
        {
          it->second.timeout_signal_connection =
              schedule_disconnection(session);
          _message(endpoint, message);
        }
    }
}

void bim::server::authentication_service::check_authentication(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)
{
  const std::optional<bim::net::authentication> message =
      bim::net::try_deserialize_message<bim::net::authentication>(m);

  const bim::net::client_token token = message->get_request_token();

  ic_log(iscool::log::nature::info(), "authentication_service",
         "Received authentication request from token %d.", token);

  if (message->get_protocol_version() != bim::net::protocol_version)
    {
      ic_log(iscool::log::nature::info(), "server",
             "Authentication request from token %d, ip=%s: bad protocol %d.",
             token, endpoint.address().to_v4().to_string(),
             message->get_protocol_version());

      m_message_stream.send(
          endpoint,
          bim::net::authentication_ko(
              token, bim::net::authentication_error_code::bad_protocol)
              .build_message());
      return;
    }

  const iscool::net::session_id session = m_next_session_id;

  session_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_sessions.emplace(token, session);

  ic_log(iscool::log::nature::info(), "server",
         "Attach session %d to token %d from ip=%s.", it->second, token,
         endpoint.address().to_v4().to_string());

  if (inserted)
    {
      ++m_next_session_id;

      client_info client{ .token = token,
                          .timeout_signal_connection =
                              schedule_disconnection(session) };

      m_clients.emplace(session, std::move(client));
    }

  m_message_stream.send(
      endpoint,
      bim::net::authentication_ok(token, it->second).build_message());
}

iscool::signals::connection
bim::server::authentication_service::schedule_disconnection(
    iscool::net::session_id session)
{
  return iscool::schedule::delayed_call(
      [this, session]() -> void
      {
        ic_log(iscool::log::nature::info(), "server", "Disconnected %d.",
               session);
        client_map::iterator it = m_clients.find(session);

        m_sessions.erase(it->second.token);
        m_clients.erase(it);
      },
      std::chrono::minutes(10));
}
