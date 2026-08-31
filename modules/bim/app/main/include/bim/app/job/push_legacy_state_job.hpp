// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim::business
{
  class request_headers;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace Json
{
  class Value;
}

namespace bim::app
{
  class analytics_service;
  struct legacy_state_transfer_response;

  class push_legacy_state_job
  {
    DECLARE_SIGNAL(void(), done, m_done)

  public:
    push_legacy_state_job(analytics_service& analytics,
                          const bim::business::request_headers& r,
                          iscool::preferences::local_preferences& preferences);
    ~push_legacy_state_job();

    void start();

  private:
    void success(const legacy_state_transfer_response& response);
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    iscool::preferences::local_preferences& m_preferences;

    iscool::http::request_connection m_http_connection;
    iscool::schedule::scoped_connection m_schedule_connection;
  };
}
