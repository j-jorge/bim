// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/tests/http_service.hpp>

#include <iscool/http/request.hpp>
#include <iscool/json/write_to_string.hpp>
#include <iscool/schedule/delayed_call.hpp>

#include <gtest/gtest.h>

bim::app::tests::http_service::http_service()
  : m_http(
        [this](iscool::http::request r) -> void
          {
            m_requests.emplace_back(std::move(r));

            if (m_process_requests_connection.connected())
              return;

            // Requests are processed with a delay in prod. Make sure we have
            // one too.
            m_process_requests_connection = iscool::schedule::delayed_call(
                [this]()
                  {
                    process_requests();
                  },
                std::chrono::seconds(0));
          })
{}

bim::app::tests::http_service::~http_service() = default;

void bim::app::tests::http_service::process_requests()
{
  m_process_requests_connection.disconnect();

  const std::vector<iscool::http::request> requests(std::move(m_requests));
  m_requests.clear();

  for (const iscool::http::request& r : requests)
    process_request(r);
}

void bim::app::tests::http_service::process_request(
    const iscool::http::request& r)
{
  // Skip the https://domain/ part of the URL.
  std::string::size_type pos = r.url.find_first_of('/');
  ASSERT_NE(std::string::npos, pos);

  pos = r.url.find_first_of('/', pos + 1);
  ASSERT_NE(std::string::npos, pos);

  pos = r.url.find_first_of('/', pos + 1);
  ASSERT_NE(std::string::npos, pos);

  const response_map::const_iterator it =
      next_response.find(r.url.substr(pos + 1));
  ASSERT_NE(next_response.end(), it);

  std::string body;
  ASSERT_TRUE(iscool::json::write_to_string(body, it->second.body));

  const int status = it->second.status;
  next_response.erase(it);

  r.result_handler({ status, std::move(body) });
}
