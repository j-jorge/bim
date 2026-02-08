// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/session_service.hpp>

#include <bim/server/config.hpp>
#include <bim/server/service/statistics_service.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/time/now.hpp>

struct bim::server::session_service::client_info
{
  boost::asio::ip::address address;
  bim::net::client_token token;
  std::chrono::nanoseconds release_at_this_date;
};

bim::server::session_service::session_service(const config& config,
                                              statistics_service& statistics)
  : m_geoloc(config)
  , m_karma(config)
  , m_statistics(statistics)
  , m_next_session_id(1)
  , m_clean_up_interval(config.session_clean_up_interval)
  , m_session_removal_delay(config.session_removal_delay)
{
  schedule_clean_up();
}

bim::server::session_service::~session_service() = default;

std::optional<iscool::net::session_id>
bim::server::session_service::create_or_refresh_session(
    const boost::asio::ip::address& address, bim::net::client_token token)
{
  if (!m_karma.allowed(address))
    return std::nullopt;

  const iscool::net::session_id session = m_next_session_id;

  session_map::const_iterator it;
  bool inserted;

  std::tie(it, inserted) = m_sessions.emplace(token, session);

  if (!inserted)
    return it->second;

  const geolocation_service::address_info address_info =
      m_geoloc.lookup(address.to_string());

  ic_log(iscool::log::nature::info(), "session_service",
         "Attach session {} to token {}, id={}, country_code={}, "
         "country='{}'.",
         it->second, token, address_info.id, address_info.country_code,
         address_info.country);

  ++m_next_session_id;

  client_info client{ .address = address,
                      .token = token,
                      .release_at_this_date = date_for_next_release() };

  m_clients.emplace(session, std::move(client));
  m_statistics.record_session_connected();

  return session;
}

bool bim::server::session_service::refresh_session(
    iscool::net::session_id session)
{
  const client_map::iterator it = m_clients.find(session);

  if (it == m_clients.end())
    return false;

  it->second.release_at_this_date = date_for_next_release();

  return true;
}

void bim::server::session_service::update_karma_disconnection(
    iscool::net::session_id session)
{
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
  std::uint64_t disconnected_count = 0;

  for (client_map::iterator it = m_clients.begin(), eit = m_clients.end();
       it != eit;)
    if (it->second.release_at_this_date <= now)
      {
        ic_log(iscool::log::nature::info(), "session_service",
               "Disconnected {}.", it->first);
        m_sessions.erase(it->second.token);
        it = m_clients.erase(it);
        ++disconnected_count;
      }
    else
      ++it;

  m_statistics.record_session_disconnected(disconnected_count);
}
