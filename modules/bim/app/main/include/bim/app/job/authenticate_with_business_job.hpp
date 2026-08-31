// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <span>

namespace Json
{
  class Value;
}

namespace bim::app
{
  class analytics_service;

  class authenticate_with_business_job
  {
    DECLARE_SIGNAL(void(std::string), done, m_done)
    DECLARE_SIGNAL(void(int), error, m_error)

  public:
    explicit authenticate_with_business_job(analytics_service& analytics);

    void start(std::string_view device_id);

  private:
    void success(const Json::Value& r);
    void error(int status, std::span<const char> body);

  private:
    analytics_service& m_analytics;

    iscool::http::request_connection m_connection;
  };
}
