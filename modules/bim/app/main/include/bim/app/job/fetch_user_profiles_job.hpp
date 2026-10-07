// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/http/request_connection_pool.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <bim/net/message/user_id.hpp>

#include <span>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  struct user_profile;
  struct user_profiles_response;

  class fetch_user_profiles_job
  {
    DECLARE_SIGNAL(void(std::span<user_profile>), done, m_done)

  public:
    fetch_user_profiles_job(analytics_service& analytics,
                            const bim::business::request_headers& r);
    ~fetch_user_profiles_job();

    void start(std::span<const bim::net::user_id> users);

  private:
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;

    iscool::http::request_connection_pool m_request_pool;
  };
}
