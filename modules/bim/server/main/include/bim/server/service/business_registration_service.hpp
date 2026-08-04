// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/shared_connection_set.hpp>

#include <chrono>
#include <string>

namespace bim::business
{
  class request_headers;
}

namespace bim::server
{
  struct config;

  class business_registration_service
  {
  public:
    business_registration_service(
        const config& config, const bim::business::request_headers& headers);
    ~business_registration_service();

  private:
    void schedule_registration(const std::chrono::seconds& delay);
    void send_registration_request();
    void hello_ko();

  private:
    std::string m_url;
    const bim::business::request_headers& m_request_headers;
    std::string m_body;

    iscool::schedule::scoped_connection m_registration_connection;
    iscool::signals::shared_connection_set m_request_connections;

    const std::chrono::seconds m_pulse;
  };
}
