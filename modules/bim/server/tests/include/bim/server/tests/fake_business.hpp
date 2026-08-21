// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/setup.hpp>
#include <iscool/schedule/scoped_connection.hpp>

#include <json/value.h>

#include <chrono>
#include <vector>

namespace iscool::http
{
  class request;
}

namespace bim::server::tests
{
  class fake_business
  {
  public:
    fake_business();
    ~fake_business();

  public:
    std::chrono::seconds hello_callback_delay;
    std::int64_t next_game_id;

    Json::Value last_game_started_request;
    Json::Value last_game_started_response;

    Json::Value last_game_over_request;

  private:
    void process_requests();
    void process_request(const iscool::http::request& r);

  private:
    const iscool::http::scoped_http_delegate m_http;
    std::vector<iscool::http::request> m_requests;
    iscool::schedule::scoped_connection m_process_requests_connection;
  };
}
