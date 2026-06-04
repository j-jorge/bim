// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <string>

namespace bim::server
{
  struct config;

  class discord_publisher_service
  {
  public:
    explicit discord_publisher_service(const config& config);
    ~discord_publisher_service();

    void send_matchmaking_notification();

  private:
    const std::string m_url;
    const std::vector<std::string> m_headers;
    const std::string m_body;
    const std::chrono::seconds m_delay_between_notifications;
    std::chrono::seconds m_date_for_next_notification;
  };
}
