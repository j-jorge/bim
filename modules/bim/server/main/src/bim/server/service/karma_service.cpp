// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/karma_service.hpp>

#include <bim/server/config.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

#include <boost/unordered/unordered_map.hpp>

struct bim::server::karma_service::client_info
{
  std::chrono::minutes let_go_at_this_date;
  std::int8_t karma;
};

bim::server::karma_service::karma_service(const config& config)
  : m_blacklist_time_out(config.karma_blacklisting_duration)
  , m_enabled(config.enable_karma)
  , m_initial_karma(config.initial_karma_value)
  , m_disconnection_karma(config.disconnection_karma_adjustment)
  , m_short_game_karma(config.short_game_karma_adjustment)
  , m_good_behavior_karma(config.good_behavior_karma_adjustment)
  , m_review_interval(config.karma_review_interval)
{
  if (m_enabled)
    {
      ic_log(iscool::log::nature::info(), "karma_service", "Started.");
      schedule_review();
    }
}

bim::server::karma_service::~karma_service() = default;

bool bim::server::karma_service::allowed(
    const boost::asio::ip::address& address) const
{
  if (!m_enabled)
    return true;

  const client_map::const_iterator it = m_client.find(address);

  return (it == m_client.end()) || (it->second.karma >= 0);
}

bim::server::karma_service::update_result
bim::server::karma_service::disconnection(
    const boost::asio::ip::address& address)
{
  return add_karma(address, m_disconnection_karma);
}

bim::server::karma_service::update_result
bim::server::karma_service::short_game(const boost::asio::ip::address& address)
{
  return add_karma(address, m_short_game_karma);
}

bim::server::karma_service::update_result
bim::server::karma_service::good_behavior(
    const boost::asio::ip::address& address)
{
  return add_karma(address, m_good_behavior_karma);
}

bim::server::karma_service::update_result
bim::server::karma_service::add_karma(const boost::asio::ip::address& address,
                                      int karma)
{
  if (!m_enabled)
    return update_result::accept;

  const client_map::iterator it =
      m_client.try_emplace(address, client_info{ .karma = m_initial_karma })
          .first;
  it->second.karma = std::max(-128, std::min(it->second.karma + karma, 127));

  if (it->second.karma >= 0)
    {
      if (karma < 0)
        ic_log(iscool::log::nature::info(), "karma_service",
               "Karma penalty of {} for {}, karma={}.", karma,
               address.to_string(), it->second.karma);

      return update_result::accept;
    }

  ic_log(iscool::log::nature::info(), "karma_service",
         "Blacklisting {} for {}, karma={}.", address.to_string(),
         m_blacklist_time_out, it->second.karma);

  const std::chrono::minutes now = iscool::time::now<std::chrono::minutes>();

  it->second.let_go_at_this_date = now + m_blacklist_time_out;

  return update_result::kick_out;
}

void bim::server::karma_service::schedule_review()
{
  m_review_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        review();
        schedule_review();
      },
      m_review_interval);
}

void bim::server::karma_service::review()
{
  const std::chrono::minutes now = iscool::time::now<std::chrono::minutes>();

  for (client_map::iterator it = m_client.begin(), eit = m_client.end();
       it != eit;)
    if ((it->second.karma < 0) && (it->second.let_go_at_this_date <= now))
      {
        ic_log(iscool::log::nature::info(), "karma_service",
               "Reopening the doors for {}.", it->first.to_string());
        it = m_client.erase(it);
      }
    else
      ++it;
}
