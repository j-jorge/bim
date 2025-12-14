// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

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
    enum class task;

  private:
    void run_thread(std::string url);

  private:
    const std::chrono::seconds m_delay_between_notifications;
    std::chrono::seconds m_date_for_next_notification;

    std::thread m_thread;
    std::condition_variable m_condition;
    std::mutex m_mutex;
    std::atomic<task> m_task;
  };
}
