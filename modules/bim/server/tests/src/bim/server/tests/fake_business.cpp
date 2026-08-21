// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_business.hpp>

#include <iscool/http/request.hpp>
#include <iscool/json/parse_string.hpp>
#include <iscool/json/write_to_string.hpp>
#include <iscool/schedule/delayed_call.hpp>

#include <json/value.h>

#include <gtest/gtest.h>

static void dispatch_ok(const iscool::http::request& r,
                        const Json::Value& body)
{
  std::string str;
  EXPECT_TRUE(iscool::json::write_to_string(str, body));
  r.result_handler(iscool::http::response{ 200, std::move(str) });
}

bim::server::tests::fake_business::fake_business()
  : hello_callback_delay(3600)
  , next_game_id(1)
  , m_http(
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
                  });
          })
{}

bim::server::tests::fake_business::~fake_business() = default;

void bim::server::tests::fake_business::process_requests()
{
  const std::vector<iscool::http::request> requests(std::move(m_requests));
  m_requests.clear();

  for (const iscool::http::request& r : requests)
    process_request(r);
}

void bim::server::tests::fake_business::process_request(
    const iscool::http::request& r)
{
  if (r.url.ends_with("/gs/game-over"))
    {
      last_game_over_request = iscool::json::parse_string(r.body);
      EXPECT_TRUE(last_game_over_request.isObject());

      r.result_handler(iscool::http::response{ 200, "{}" });
      return;
    }

  if (r.url.ends_with("/gs/game-started"))
    {
      last_game_started_request = iscool::json::parse_string(r.body);
      EXPECT_TRUE(last_game_started_request.isObject());

      last_game_started_response = Json::Value(Json::objectValue);
      last_game_started_response["game_id"] = next_game_id;
      ++next_game_id;

      dispatch_ok(r, last_game_started_response);
      return;
    }

  if (r.url.ends_with("/gs/hello"))
    {
      Json::Value response;
      response["callback_delay_seconds"] = (int)hello_callback_delay.count();
      dispatch_ok(r, response);
      return;
    }

  if (r.url.ends_with("/gs/user-id"))
    {
      const Json::Value body = iscool::json::parse_string(r.body);
      EXPECT_TRUE(body.isObject());

      Json::Value response;
      response["tokens"] = Json::arrayValue;
      response["user_ids"] = Json::arrayValue;

      for (Json::ArrayIndex i = 0, n = body["sessions"].size(); i != n; ++i)
        {
          response["tokens"].append(body["tokens"][i]);
          response["user_ids"][i] = std::stol(body["sessions"][i].asString());
        }

      dispatch_ok(r, response);
      return;
    }
}
