// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/schedule/scoped_connection.hpp>

#include <boost/asio/ip/address.hpp>
#include <boost/unordered/unordered_map.hpp>

#include <chrono>
#include <cstdint>

namespace bim::server
{
  struct config;

  class karma_service
  {
  public:
    enum class update_result
    {
      accept,
      kick_out
    };

  public:
    explicit karma_service(const config& config);
    ~karma_service();

    bool allowed(const boost::asio::ip::address& address) const;

    update_result disconnection(const boost::asio::ip::address& address);
    update_result short_game(const boost::asio::ip::address& address);
    update_result good_behavior(const boost::asio::ip::address& address);

  private:
    struct client_info;

    using client_map =
        boost::unordered_map<boost::asio::ip::address, client_info,
                             std::hash<boost::asio::ip::address>>;

  private:
    update_result add_karma(const boost::asio::ip::address& address,
                            int karma);

    void schedule_review();
    void review();

  private:
    client_map m_client;

    const std::chrono::minutes m_blacklist_time_out;
    const bool m_enabled;
    const std::int8_t m_initial_karma;
    const std::int8_t m_disconnection_karma;
    const std::int8_t m_short_game_karma;
    const std::int8_t m_good_behavior_karma;

    iscool::schedule::scoped_connection m_review_connection;
    std::chrono::minutes m_review_interval;
  };
}
