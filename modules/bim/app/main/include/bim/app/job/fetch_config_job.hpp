// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/request_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace Json
{
  class Value;
}

namespace bim::app
{
  class analytics_service;
  struct config;

  class fetch_config_job
  {
    DECLARE_VOID_SIGNAL(done, m_done)

  public:
    explicit fetch_config_job(analytics_service& analytics);

    void start(config& cfg);

  private:
    void validate_remote_config(const Json::Value& json_config);
    void load_local_config();
    void config_ready();

  private:
    analytics_service& m_analytics;
    config* m_config;

    iscool::http::request_connection m_connection;
  };
}
