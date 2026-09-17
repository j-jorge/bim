// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  class player_profile;
  class update_nickname_response;

  class update_nickname_job
  {
    DECLARE_SIGNAL(void(), done, m_done)
    DECLARE_SIGNAL(void(), error, m_error)

  public:
    update_nickname_job(analytics_service& analytics,
                        const bim::business::request_headers& r,
                        player_profile& p);
    ~update_nickname_job();

    void start(std::string nickname);

  private:
    void success(std::string nickname, const update_nickname_response& r);
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    player_profile& m_player_profile;

    iscool::http::request_connection m_connection;
  };
}
