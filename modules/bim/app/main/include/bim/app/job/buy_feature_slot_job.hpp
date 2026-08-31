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
  struct config;
  class player_profile;

  class buy_feature_slot_job
  {
    DECLARE_SIGNAL(void(std::size_t), done, m_done)

  public:
    buy_feature_slot_job(analytics_service& analytics,
                         const bim::business::request_headers& r,
                         const config& cfg, player_profile& p);
    ~buy_feature_slot_job();

    void start(std::size_t i);

  private:
    void success(std::size_t i);
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    const config& m_config;
    player_profile& m_player_profile;

    iscool::http::request_connection m_connection;
  };
}
