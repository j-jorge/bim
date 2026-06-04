// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/discord_publisher_service.hpp>

#include <bim/server/config.hpp>

#include <bim/assume.hpp>

#include <iscool/http/send.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/time/now.hpp>

bim::server::discord_publisher_service::discord_publisher_service(
    const config& config)
  : m_url(config.enable_discord_matchmaking_notifications
              ? config.discord_matchmaking_notification_url
              : "")
  , m_headers({ "Content-Type: application/json" })
  , m_body("{\"content\":\"A player is looking for an opponent! Go!\"}")
  , m_delay_between_notifications(
        config.discord_matchmaking_notification_interval)
  , m_date_for_next_notification(0)
{
  if (config.enable_discord_matchmaking_notifications)
    ic_log(iscool::log::nature::info(), "discord_publisher_service",
           "Starting.");
  else
    ic_log(iscool::log::nature::info(), "discord_publisher_service",
           "Disabled.");
}

bim::server::discord_publisher_service::~discord_publisher_service() = default;

void bim::server::discord_publisher_service::send_matchmaking_notification()
{
  if (m_url.empty())
    return;

  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  if (now < m_date_for_next_notification)
    return;

  m_date_for_next_notification = now + m_delay_between_notifications;

  iscool::http::post(m_url, m_headers, m_body, {}, {});
}
