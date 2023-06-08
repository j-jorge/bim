/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bm/server/service/authentication_service.hpp>

#include <bm/net/message/authentication.hpp>
#include <bm/net/message/authentication_ko.hpp>
#include <bm/net/message/authentication_ok.hpp>
#include <bm/net/message/protocol_version.hpp>

#include <iscool/log/causeless_log.h>
#include <iscool/log/nature/info.h>
#include <iscool/schedule/delayed_call.h>
#include <iscool/signals/implement_signal.h>
#include <iscool/signals/scoped_connection.h>

IMPLEMENT_SIGNAL(bm::server::authentication_service, message, _message);

struct bm::server::authentication_service::client_info
{
  bm::net::client_token token;
  iscool::signals::scoped_connection timeout_signal_connection;
};

bm::server::authentication_service::authentication_service(
    iscool::net::socket_stream& socket)
  : m_message_stream(socket)
  , m_next_session_id(1)
{
  m_message_stream.connect_to_message(
      std::bind(&authentication_service::check_session, this,
                std::placeholders::_1, std::placeholders::_2));
}

bm::server::authentication_service::~authentication_service() = default;

void bm::server::authentication_service::check_session(
    const iscool::net::endpoint& endpoint, const iscool::net::message& message)
{
  const iscool::net::session_id session = message.get_session_id();

  if (session == 0)
    {
      if (message.get_type() == bm::net::message_type::authentication)
        check_authentication(endpoint,
                             bm::net::authentication(message.get_content()));
    }
  else
    {
      const client_map::iterator it = m_clients.find(session);

      if (it != m_clients.end())
        {
          it->second.timeout_signal_connection
              = schedule_disconnection(session);
          _message(endpoint, message);
        }
    }
}

void bm::server::authentication_service::check_authentication(
    const iscool::net::endpoint& endpoint,
    const bm::net::authentication& message)
{
  const bm::net::client_token token = message.get_request_token();

  ic_causeless_log(iscool::log::nature::info(), "authentication_service",
                   "Received authentication request from token %d.", token);

  if (message.get_protocol_version() != bm::net::protocol_version)
    {
      ic_causeless_log(
          iscool::log::nature::info(), "server",
          "Authentication request from token %d: bad protocol %d.", token,
          message.get_protocol_version());

      m_message_stream.send(
          endpoint,
          bm::net::authentication_ko(
              token, bm::net::authentication_error_code::bad_protocol)
              .build_message());
      return;
    }

  const iscool::net::session_id session = m_next_session_id;

  session_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_sessions.emplace(token, session);

  ic_causeless_log(iscool::log::nature::info(), "server",
                   "Attach session %d to token %d.", it->second, token);

  if (inserted)
    {
      ++m_next_session_id;

      client_info client{ .token = token,
                          .timeout_signal_connection
                          = schedule_disconnection(session) };

      m_clients.emplace(session, std::move(client));
    }

  m_message_stream.send(
      endpoint, bm::net::authentication_ok(token, it->second).build_message());
}

iscool::signals::connection
bm::server::authentication_service::schedule_disconnection(
    iscool::net::session_id session)
{
  return iscool::schedule::delayed_call(
      [this, session]() -> void
      {
        ic_causeless_log(iscool::log::nature::info(), "server",
                         "Disconnected %d.", session);
        client_map::iterator it = m_clients.find(session);

        m_sessions.erase(it->second.token);
        m_clients.erase(it);
      },
      std::chrono::minutes(10));
}
