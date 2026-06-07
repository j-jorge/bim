// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/shared_connection_set.hpp>

#include <chrono>
#include <span>
#include <string>
#include <vector>

namespace bim::server
{
  struct config;

  class business_registration_service
  {
  public:
    explicit business_registration_service(const config& config);
    ~business_registration_service();

  private:
    void schedule_registration(const std::chrono::seconds& delay);
    void send_registration_request();
    void hello_ok(std::span<const char> body);
    void hello_ko(std::span<const char> body);
    void increment_retry_delay();

  private:
    std::string m_url;
    std::vector<std::string> m_headers;
    std::string m_body;

    iscool::schedule::scoped_connection m_registration_connection;
    iscool::signals::shared_connection_set m_request_connections;

    std::chrono::seconds m_retry_delay;
  };
}
