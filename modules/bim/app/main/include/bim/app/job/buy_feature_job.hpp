// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/http/request_connection_pool.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <bim/game/feature_flags_fwd.hpp>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  struct config;
  class player_profile;

  class buy_feature_job
  {
    DECLARE_SIGNAL(void(bim::game::feature_flags), done, m_done)

  public:
    buy_feature_job(analytics_service& analytics,
                    const bim::business::request_headers& r, const config& cfg,
                    player_profile& p);
    ~buy_feature_job();

    void start(bim::game::feature_flags f);

  private:
    void success(bim::game::feature_flags f);
    void error();

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    const config& m_config;
    player_profile& m_player_profile;

    iscool::http::request_connection_pool m_request_pool;
  };
}
