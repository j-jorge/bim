// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/session_handler.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <cassert>

IMPLEMENT_SIGNAL(bim::net::session_handler, connected, m_connected);
IMPLEMENT_SIGNAL(bim::net::session_handler, authentication_error,
                 m_authentication_error);

bim::net::session_handler::session_handler()
  : m_message_stream(m_socket_stream)
  , m_authentication(m_message_stream)
{
  m_authentication_connection = m_authentication.connect_to_authenticated(
      [this](iscool::net::session_id session) -> void
      {
        m_session_id = session;
        m_connected();
      });
  m_authentication_error_connection = m_authentication.connect_to_error(
      [this](authentication_error_code c) -> void
      {
        m_session_id = std::nullopt;
        m_authentication_error(c);
      });
}

bim::net::session_handler::~session_handler() = default;

void bim::net::session_handler::connect(std::string_view host)
{
  m_authentication.stop();
  m_session_id = std::nullopt;

  try
    {
      m_socket_stream.connect(std::string(host));
    }
  catch (const std::exception& e)
    {
      ic_log(iscool::log::nature::error(), "session_handler",
             "Failed to open socket: {}", e.what());
      return;
    }

  m_authentication.start();
}

const iscool::net::message_stream&
bim::net::session_handler::message_stream() const
{
  return m_message_stream;
}

bool bim::net::session_handler::connected() const
{
  return !!m_session_id;
}

iscool::net::session_id bim::net::session_handler::session_id() const
{
  assert(m_session_id);
  return *m_session_id;
}
