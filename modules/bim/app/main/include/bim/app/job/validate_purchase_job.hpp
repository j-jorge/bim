// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/business/purchase_validation_status_fwd.hpp>

#include <iscool/http/request_connection.hpp>
#include <iscool/http/request_connection_pool.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <string>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  class player_profile;
  struct validate_purchase_response;

  class validate_purchase_job
  {
    DECLARE_SIGNAL(void(purchase_validation_status), done, m_done)
    DECLARE_SIGNAL(void(), error, m_error)

  public:
    validate_purchase_job(analytics_service& analytics,
                          const bim::business::request_headers& r,
                          player_profile& p);
    ~validate_purchase_job();

    void start(std::string purchase_token);

  private:
    void success(const validate_purchase_response& response);
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    player_profile& m_player_profile;

    iscool::http::request_connection_pool m_request_pool;
  };
}
