// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/discord_publisher_service.hpp>

#include <bim/server/config.hpp>

#include <bim/assume.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/time/now.hpp>

#include <curl/curl.h>

enum class bim::server::discord_publisher_service::task
{
  wait,
  send,
  quit
};

bim::server::discord_publisher_service::discord_publisher_service(
    const config& config)
  : m_delay_between_notifications(
        config.discord_matchmaking_notification_interval)
  , m_date_for_next_notification(0)
  , m_task(task::wait)
{
  if (!config.enable_discord_matchmaking_notifications)
    return;

  ic_log(iscool::log::nature::info(), "discord_publisher_service",
         "Starting.");

  m_thread = std::thread(&discord_publisher_service::run_thread, this,
                         config.discord_matchmaking_notification_url);
}

bim::server::discord_publisher_service::~discord_publisher_service()
{
  if (!m_thread.joinable())
    return;

  {
    const std::unique_lock<std::mutex> lock(m_mutex);
    m_task = task::quit;
    m_condition.notify_one();
  }

  m_thread.join();
}

void bim::server::discord_publisher_service::send_matchmaking_notification()
{
  const std::chrono::seconds now = iscool::time::now<std::chrono::seconds>();

  if (now < m_date_for_next_notification)
    return;

  m_date_for_next_notification = now + m_delay_between_notifications;

  const std::unique_lock<std::mutex> lock(m_mutex);
  m_task = task::send;
  m_condition.notify_one();
}

void bim::server::discord_publisher_service::run_thread(std::string url)
{
  CURL* const curl = curl_easy_init();
  CURLcode result;
  curl_slist* headers = nullptr;

  if (!curl)
    ic_log(iscool::log::nature::error(), "discord_publisher_service",
           "Failed to create CURL handle.");
  else
    {
      result = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

      if (result != CURLE_OK)
        ic_log(iscool::log::nature::error(), "discord_publisher_service",
               "Failed to set URL: {}", curl_easy_strerror(result));

      headers = curl_slist_append(nullptr, "Content-Type: application/json");
      result = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

      if (result != CURLE_OK)
        ic_log(iscool::log::nature::error(), "discord_publisher_service",
               "Failed to set HTTP headers: {}", curl_easy_strerror(result));

      result = curl_easy_setopt(
          curl, CURLOPT_POSTFIELDS,
          "{\"content\":\"A player is looking for an opponent! Go!\"}");

      if (result != CURLE_OK)
        ic_log(iscool::log::nature::error(), "discord_publisher_service",
               "Failed to set POST fields: {}", curl_easy_strerror(result));
    }

  bool quit = false;
  while (!quit)
    {
      task t;

      {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock,
                         [this]()
                         {
                           return m_task != task::wait;
                         });
        t = m_task.exchange(task::wait);
      }

      switch (t)
        {
        case task::quit:
          quit = true;
          break;
        case task::send:
          {
            result = curl_easy_perform(curl);

            if (result != CURLE_OK)
              ic_log(iscool::log::nature::error(), "discord_publisher_service",
                     "curl_easy_perform() failed: {}",
                     curl_easy_strerror(result));

            break;
          }
        case task::wait:
          bim_assume(false);
        }
    }

  if (curl)
    curl_easy_cleanup(curl);

  if (headers)
    curl_slist_free_all(headers);
}
