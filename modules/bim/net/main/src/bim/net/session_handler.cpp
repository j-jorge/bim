#include <bim/net/session_handler.hpp>

#include <iscool/http/send.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/is_of_type_string.hpp>
#include <iscool/json/parse_string.hpp>
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

void bim::net::session_handler::connect(const std::string& config_url)
{
  m_authentication.stop();
  m_session_id = std::nullopt;

  auto on_result = [this](const std::vector<char>& response) -> void
  {
    connect_to_game_server(response);
  };

  auto on_error = [this](const std::vector<char>& response) -> void
  {
    dispatch_error(response);
  };

  m_config_request_connections =
      iscool::http::get(config_url, on_result, on_error);
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

void bim::net::session_handler::connect_to_game_server(
    const std::vector<char>& response)
{
  const Json::Value config = iscool::json::parse_string(
      std::string(response.begin(), response.end()));

  if (!config.isObject())
    {
      m_config_error();
      return;
    }

  const Json::Value& host = config["host"];

  if (!iscool::json::is_of_type<std::string>(host))
    {
      m_config_error();
      return;
    }

  m_socket_stream.connect(iscool::json::cast<std::string>(host));
  m_authentication.start();
}

void bim::net::session_handler::dispatch_error(
    const std::vector<char>& response) const
{
  const std::string_view text(response.begin(), response.end());

  ic_log(iscool::log::nature::error(), "session_handler",
         "Failed to fetch the game server config: %s", text);

  m_config_error();
}
