// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim::business
{
  class request_headers;
}

namespace Json
{
  class Value;
}

namespace bim::app
{
  class analytics_service;
  class player_profile;

  class fetch_player_profile_job
  {
    DECLARE_SIGNAL(void(), done, m_done)

  public:
    fetch_player_profile_job(analytics_service& analytics,
                             const bim::business::request_headers& r);
    ~fetch_player_profile_job();

    void start(player_profile& profile);

  private:
    void success();
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;

    player_profile* m_player_profile;

    iscool::http::request_connection m_connection;
  };
}
