// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/session_service.hpp>

#include <bim/server/config.hpp>
#include <bim/server/service/statistics_service.hpp>

#include <iscool/http/json/send.hpp>
#include <iscool/json/cast_int64.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/time/now.hpp>

#include <cassert>

struct bim::server::session_service::client_info
{
  boost::asio::ip::address address;
  bim::net::client_token token;
  std::chrono::nanoseconds release_at_this_date;
  bim::net::user_id user_id;
  bim::net::session_token session_token;
};

static constexpr iscool::net::session_id g_bot_min_session =
    std::numeric_limits<iscool::net::session_id>::max() / 2 + 1;

IMPLEMENT_SIGNAL(bim::server::session_service, sessions_ready,
                 m_sessions_ready);

bim::server::session_service::session_service(const config& config,
                                              statistics_service& statistics)
  : m_geoloc(config)
  , m_karma(config)
  , m_statistics(statistics)
  , m_next_real_session_id(1)
  , m_next_bot_session_id(g_bot_min_session)
  , m_clean_up_interval(config.session_clean_up_interval)
  , m_session_removal_delay(config.session_removal_delay)
  , m_user_id_url(
        config.business_url.empty() ? "" : config.business_url + "gs/user-id")
  , m_ongoing_user_id_business_request(false)
{
  if (!m_user_id_url.empty())
    m_headers.emplace_back("Authorization: " + config.business_token);

  schedule_clean_up();
}

bim::server::session_service::~session_service() = default;

iscool::net::session_id bim::server::session_service::new_bot_session()
{
  const iscool::net::session_id result = m_next_bot_session_id;

  if (m_next_bot_session_id
      == std::numeric_limits<iscool::net::session_id>::max())
    {
      ic_log(iscool::log::nature::info(), "session_service",
             "Max bot session reached, looping.");
      m_next_bot_session_id = g_bot_min_session;
    }
  else
    ++m_next_bot_session_id;

  return result;
}

bim::server::create_session_result
bim::server::session_service::create_or_refresh_session(
    const boost::asio::ip::address& address, bim::net::client_token token,
    const bim::net::session_token& session_token)
{
  if (!m_karma.allowed(address))
    return { create_session_result_state::rejected, token, 0 };

  if (m_next_real_session_id == g_bot_min_session)
    {
      ic_log(iscool::log::nature::info(), "session_service",
             "Max session reached, no new session can be created.");
      return { create_session_result_state::rejected, token, 0 };
    }

  const iscool::net::session_id session = m_next_real_session_id;

  session_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_sessions.emplace(token, session);

  if (!inserted)
    {
      if (m_user_id_url.empty())
        return { create_session_result_state::accepted, token, it->second };

      const client_map::const_iterator client_it = m_clients.find(it->second);

      if (session_token != client_it->second.session_token)
        {
          ic_log(iscool::log::nature::info(), "session_service",
                 "Unexpected same client token for different session tokens.");
          return { create_session_result_state::rejected, token, 0 };
        }

      return { (client_it->second.user_id == 0)
                   ? create_session_result_state::pending
                   : create_session_result_state::accepted,
               token, it->second };
    }

  const geolocation_service::address_info address_info =
      m_geoloc.lookup(address.to_string());

  for (std::size_t i = 0, n = session_token.size(); i != n; ++i)
    {
      const char c = session_token[i];

      if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))
            || ((c >= '0') && (c <= '9')) || (c == '+') || (c == '/')
            || (c == '=')))
        {
          ic_log(iscool::log::nature::error(), "session_service",
                 "Token does not seems base64-encoded. Character {} is {}.", i,
                 (int)c);
          return { create_session_result_state::rejected, token, 0 };
        }
    }

  std::string session_token_str((const char*)session_token.data(),
                                session_token.size());

  ic_log(iscool::log::nature::info(), "session_service",
         "Attach session {} to token {}, id={}, country_code={}, "
         "country='{}', session_token={}.",
         session, token, address_info.id, address_info.country_code,
         address_info.country, session_token_str);

  ++m_next_real_session_id;

  client_info client{ .address = address,
                      .token = token,
                      .release_at_this_date = date_for_next_release(),
                      .user_id = 0,
                      .session_token = session_token };

  m_clients.emplace(session, std::move(client));
  m_statistics.record_session_connected();
  m_karma.add(address);

  if (m_user_id_url.empty())
    return { create_session_result_state::accepted, token, session };

  m_user_id_business_request["tokens"].append(token);
  m_user_id_business_request["sessions"].append(session_token_str);

  schedule_user_id_request();

  return { create_session_result_state::pending, token, 0 };
}

bool bim::server::session_service::refresh_session(
    iscool::net::session_id session)
{
  assert(session < g_bot_min_session);

  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end()
      || (!m_user_id_url.empty() && (it->second.user_id == 0)))
    return false;

  it->second.release_at_this_date = date_for_next_release();

  return true;
}

bim::net::user_id
bim::server::session_service::user_id(iscool::net::session_id session) const
{
  const client_map::const_iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return 0;

  return it->second.user_id;
}

void bim::server::session_service::update_karma_disconnection(
    iscool::net::session_id session)
{
  assert(session < g_bot_min_session);

  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  ic_log(iscool::log::nature::info(), "session_service",
         "Internal disconnection for session={}.", session);

  m_karma.disconnection(it->second.address);
  disconnect(it);
}

void bim::server::session_service::update_karma_short_game(
    iscool::net::session_id session)
{
  assert(session < g_bot_min_session);

  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  if (m_karma.short_game(it->second.address)
      == karma_service::update_result::kick_out)
    disconnect(it);
}

void bim::server::session_service::update_karma_good_behavior(
    iscool::net::session_id session)
{
  assert(session < g_bot_min_session);

  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return;

  m_karma.good_behavior(it->second.address);
}

std::chrono::nanoseconds
bim::server::session_service::date_for_next_release() const
{
  return iscool::time::now<std::chrono::nanoseconds>()
         + m_session_removal_delay;
}

void bim::server::session_service::disconnect(const client_map::iterator& it)
{
  m_sessions.erase(it->second.token);
  m_clients.erase(it);

  m_statistics.record_session_disconnected(1);
}

void bim::server::session_service::schedule_clean_up()
{
  m_clean_up_connection = iscool::schedule::delayed_call(
      [this]() -> void
        {
          clean_up();
          schedule_clean_up();
        },
      m_clean_up_interval);
}

void bim::server::session_service::clean_up()
{
  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();

  const std::size_t old_client_count = m_clients.size();

  for (client_map::iterator it = m_clients.begin(), eit = m_clients.end();
       it != eit;)
    if (it->second.release_at_this_date <= now)
      {
        ic_log(iscool::log::nature::info(), "session_service",
               "Disconnected {}.", it->first);
        m_sessions.erase(it->second.token);
        m_karma.remove(it->second.address);
        it = m_clients.erase(it);
      }
    else
      ++it;

  if (old_client_count != m_clients.size())
    {
      ic_log(iscool::log::nature::info(), "session_service",
             "Session clean up {} -> {}.", old_client_count, m_clients.size());

      m_statistics.record_session_disconnected(old_client_count
                                               - m_clients.size());
    }
}

void bim::server::session_service::schedule_user_id_request()
{
  if (m_schedule_user_id_connection.connected()
      || !m_user_id_connections.empty()
      || m_user_id_business_request["tokens"].empty())
    return;

  m_schedule_user_id_connection = iscool::schedule::delayed_call(
      [this]()
        {
          m_schedule_user_id_connection.disconnect();
          fetch_user_ids();
        });
}

void bim::server::session_service::fetch_user_ids()
{
  m_user_id_connections = iscool::http::json::post(
      m_user_id_url, m_headers, m_user_id_business_request,
      [this](const Json::Value& r)
        {
          user_id_response(r);
        },
      [this](int code, std::span<const char> b)
        {
          user_id_error(code, b);
        });

  m_user_id_business_request["tokens"].clear();
  m_user_id_business_request["sessions"].clear();
}

void bim::server::session_service::user_id_response(
    const Json::Value& response)
{
  m_create_session_dispatch.clear();

  const Json::Value& tokens = response["tokens"];
  const Json::Value& users = response["user_ids"];

  for (Json::ArrayIndex i = 0, n = tokens.size(); i != n; ++i)
    {
      const bim::net::client_token token(
          iscool::json::cast<bim::net::client_token>(tokens[i]));

      const Json::Value& user = users[i];

      if (user == Json::nullValue)
        {
          m_create_session_dispatch.push_back(
              { create_session_result_state::rejected, token, 0 });
          continue;
        }

      const bim::net::user_id user_id(
          iscool::json::cast<bim::net::user_id>(users[i]));
      assert(user_id != 0);

      const session_map::const_iterator session_it = m_sessions.find(token);

      if (session_it == m_sessions.end())
        {
          ic_log(iscool::log::nature::info(), "session_service",
                 "Got user ID {} for unknown token {}.", (std::int64_t)user_id,
                 token);
          continue;
        }

      const client_map::iterator client_it =
          m_clients.find(session_it->second);
      assert(client_it != m_clients.end());

      client_it->second.user_id = user_id;
      m_create_session_dispatch.push_back(
          { create_session_result_state::accepted, token,
            session_it->second });
    }

  m_ongoing_user_id_business_request = false;

  m_user_id_connections.clear();
  schedule_user_id_request();

  if (!m_create_session_dispatch.empty())
    m_sessions_ready(m_create_session_dispatch);
}

void bim::server::session_service::user_id_error(int code,
                                                 std::span<const char> body)
{
  ic_log(iscool::log::nature::info(), "session_service",
         "Failed to fetch user IDs {}: {}", code,
         std::string_view(body.data(), body.size()));
  m_ongoing_user_id_business_request = false;

  m_user_id_connections.clear();
  schedule_user_id_request();
}
