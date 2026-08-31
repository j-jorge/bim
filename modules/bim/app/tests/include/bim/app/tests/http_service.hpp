// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/http/setup.hpp>
#include <iscool/schedule/scoped_connection.hpp>

#include <json/value.h>

#include <vector>

namespace Json
{
  class Value;
}

namespace bim::app::tests
{
  class http_service
  {
  public:
    struct response
    {
      int status;
      Json::Value body;
    };

    using response_map = std::unordered_map<std::string, response>;

  public:
    http_service();
    ~http_service();

  private:
    void process_requests();
    void process_request(const iscool::http::request& r);

  public:
    std::unordered_map<std::string, Json::Value> last_request;
    response_map next_response;

  private:
    const iscool::http::scoped_http_delegate m_http;
    std::vector<iscool::http::request> m_requests;
    iscool::schedule::scoped_connection m_process_requests_connection;
  };
}
