// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/session_service.hpp>

#include <bim/server/service/statistics_service.hpp>

#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>

#include <iscool/http/request.hpp>
#include <iscool/http/setup.hpp>
#include <iscool/json/parse_string.hpp>

#include <chrono>

#include <gtest/gtest.h>

TEST(session_service, sessions_and_tokens)
{
  bim::server::tests::fake_scheduler scheduler;

  bim::server::config config = bim::server::tests::new_test_config();
  config.session_clean_up_interval = std::chrono::seconds(1);
  config.session_removal_delay = std::chrono::seconds(5);

  bim::server::statistics_service statistics(config);
  bim::server::session_service service(config, statistics);

  const boost::asio::ip::address_v4 address_0(0x01010101);
  const boost::asio::ip::address_v4 address_1(0x02020202);

  const bim::server::create_session_result session_0 =
      service.create_or_refresh_session(address_0, 111, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_0.state);

  const bim::server::create_session_result session_1 =
      service.create_or_refresh_session(address_1, 222, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_1.state);

  const bim::server::create_session_result session_0_bis =
      service.create_or_refresh_session(address_0, 111, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_0_bis.state);
  EXPECT_EQ(session_0.session, session_0_bis.session);

  const bim::server::create_session_result session_0_ter =
      service.create_or_refresh_session(address_1, 111, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_0_ter.state);
  EXPECT_EQ(session_0.session, session_0_ter.session);

  // Trigger the clean up but keep session 1 active, such that only session 0
  // is removed.
  for (int i = 0; i != 6; ++i)
    {
      EXPECT_TRUE(service.refresh_session(session_1.session));
      scheduler.tick(std::chrono::seconds(1));
    }

  // session_0 has been removed, same address and token pair creates a new
  // session.
  const bim::server::create_session_result new_session_0 =
      service.create_or_refresh_session(address_0, 111, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            new_session_0.state);
  EXPECT_NE(session_0.session, new_session_0.session);

  // session_1 is still active so the same token should return the same
  // session.
  const bim::server::create_session_result session_1_bis =
      service.create_or_refresh_session(address_1, 222, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_1_bis.state);
  EXPECT_EQ(session_1.session, session_1_bis.session);
}

TEST(session_service, karma)
{
  bim::server::tests::fake_scheduler scheduler;

  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_karma = true;
  config.session_clean_up_interval = std::chrono::hours(50);
  config.session_removal_delay = std::chrono::hours(50);
  config.karma_blacklisting_duration = std::chrono::minutes(10);
  config.karma_review_interval = std::chrono::minutes(5);
  config.initial_karma_value = 10;
  config.disconnection_karma_adjustment = -6;
  config.short_game_karma_adjustment = -3;
  config.good_behavior_karma_adjustment = 3;

  bim::server::statistics_service statistics(config);
  bim::server::session_service service(config, statistics);

  const boost::asio::ip::address_v4 a_1(0x01010101);
  const boost::asio::ip::address_v4 a_2(0x02020202);
  const boost::asio::ip::address_v4 a_3(0x03030303);

  bim::server::create_session_result session_1 =
      service.create_or_refresh_session(a_1, 111, {});
  bim::server::create_session_result session_2 =
      service.create_or_refresh_session(a_2, 222, {});
  const bim::server::create_session_result session_3 =
      service.create_or_refresh_session(a_3, 333, {});

  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_1.state);
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_2.state);
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_3.state);

  service.update_karma_disconnection(session_1.session);
  // karma = 7
  service.update_karma_short_game(session_2.session);

  EXPECT_FALSE(service.refresh_session(session_1.session));
  EXPECT_TRUE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_1.state);

  service.update_karma_disconnection(session_1.session);
  // karma = 4
  service.update_karma_short_game(session_2.session);

  EXPECT_FALSE(service.refresh_session(session_1.session));
  EXPECT_TRUE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_1.state);

  // Update the karma service, nothing changes.
  scheduler.tick(std::chrono::minutes(5));

  EXPECT_TRUE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_1.state);

  // karma = 1
  service.update_karma_short_game(session_2.session);

  EXPECT_TRUE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_1.state);

  // karma back to 4
  service.update_karma_good_behavior(session_2.session);

  EXPECT_TRUE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_1.state);

  // karma = 1
  service.update_karma_short_game(session_2.session);

  EXPECT_TRUE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_1.state);

  // karma = -2
  service.update_karma_short_game(session_2.session);

  EXPECT_FALSE(service.refresh_session(session_2.session));
  EXPECT_TRUE(service.refresh_session(session_3.session));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  session_2 = service.create_or_refresh_session(a_2, 555, {});

  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_1.state);
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_2.state);

  // Update the service, a_1 is back.
  scheduler.tick(std::chrono::minutes(5));

  session_1 = service.create_or_refresh_session(a_1, 444, {});
  session_2 = service.create_or_refresh_session(a_2, 555, {});

  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_1.state);
  EXPECT_EQ(bim::server::create_session_result_state::rejected,
            session_2.state);
  EXPECT_TRUE(service.refresh_session(session_3.session));

  // Update the service, a_2 is back.
  scheduler.tick(std::chrono::minutes(5));

  session_2 = service.create_or_refresh_session(a_2, 555, {});

  EXPECT_TRUE(service.refresh_session(session_1.session));
  EXPECT_EQ(bim::server::create_session_result_state::accepted,
            session_2.state);
  EXPECT_TRUE(service.refresh_session(session_3.session));
}

TEST(session_service, user_id)
{
  bim::server::tests::fake_scheduler scheduler;
  std::optional<iscool::http::request> last_http_request;

  const iscool::http::scoped_http_delegate http(
      [&](iscool::http::request r) -> void
        {
          last_http_request = std::move(r);
        });

  bim::server::config config = bim::server::tests::new_test_config();
  config.session_clean_up_interval = std::chrono::seconds(1);
  config.session_removal_delay = std::chrono::seconds(5);
  config.business_url = "biz/";

  bim::server::statistics_service statistics(config);
  bim::server::session_service service(config, statistics);

  // Pretend some users want to connect.
  const boost::asio::ip::address_v4 address_1(0x01010101);
  const boost::asio::ip::address_v4 address_2(0x02020202);
  const boost::asio::ip::address_v4 address_3(0x03030303);
  const boost::asio::ip::address_v4 address_4(0x04040404);

  const std::string session_token_1 = "st1";
  const std::string session_token_2 = "st2";
  const std::string session_token_3 = "st3";
  const std::string session_token_4 = "st4";

  const bim::server::create_session_result session_1 =
      service.create_or_refresh_session(
          address_1, 111,
          bim::net::session_token(session_token_1.begin(),
                                  session_token_1.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_1.state);

  const bim::server::create_session_result session_2 =
      service.create_or_refresh_session(
          address_2, 222,
          bim::net::session_token(session_token_2.begin(),
                                  session_token_2.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_2.state);

  const bim::server::create_session_result session_3 =
      service.create_or_refresh_session(
          address_3, 333,
          bim::net::session_token(session_token_3.begin(),
                                  session_token_3.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_3.state);

  const bim::server::create_session_result session_2_bis =
      service.create_or_refresh_session(
          address_2, 222,
          bim::net::session_token(session_token_2.begin(),
                                  session_token_2.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_2_bis.state);

  const bim::server::create_session_result session_2_ter =
      service.create_or_refresh_session(
          address_2, 2222,
          bim::net::session_token(session_token_2.begin(),
                                  session_token_2.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_2_ter.state);

  // The service should have queued the requests to send them all at once to
  // the business server. Updating the scheduler should trigger the query to
  // the business.
  scheduler.tick(std::chrono::seconds(1));
  ASSERT_TRUE(!!last_http_request);
  EXPECT_EQ("biz/gs/user-id", last_http_request->url);

  Json::Value body = iscool::json::parse_string(last_http_request->body);
  EXPECT_TRUE(body.isObject());
  ASSERT_EQ(4, body["sessions"].size());
  EXPECT_EQ(session_token_1, body["sessions"][0].asString());
  EXPECT_EQ(session_token_2, body["sessions"][1].asString());
  EXPECT_EQ(session_token_3, body["sessions"][2].asString());
  EXPECT_EQ(session_token_2, body["sessions"][3].asString());

  ASSERT_EQ(4, body["tokens"].size());
  EXPECT_EQ(111, body["tokens"][0].asInt());
  EXPECT_EQ(222, body["tokens"][1].asInt());
  EXPECT_EQ(333, body["tokens"][2].asInt());
  EXPECT_EQ(2222, body["tokens"][3].asInt());

  bool sessions_ready = false;
  iscool::signals::connection connection = service.connect_to_sessions_ready(
      [&](std::span<const bim::server::create_session_result> results)
        {
          sessions_ready = true;
          ASSERT_EQ(4, results.size());

          EXPECT_EQ(bim::server::create_session_result_state::accepted,
                    results[0].state);
          EXPECT_EQ(222, results[0].token);
          EXPECT_EQ(2, results[0].session);
          EXPECT_EQ(202, service.user_id(results[0].session));

          EXPECT_EQ(bim::server::create_session_result_state::accepted,
                    results[1].state);
          EXPECT_EQ(111, results[1].token);
          EXPECT_EQ(1, results[1].session);
          EXPECT_EQ(101, service.user_id(results[1].session));

          EXPECT_EQ(bim::server::create_session_result_state::rejected,
                    results[2].state);
          EXPECT_EQ(333, results[2].token);

          EXPECT_EQ(bim::server::create_session_result_state::accepted,
                    results[3].state);
          EXPECT_EQ(2222, results[3].token);
          EXPECT_EQ(4, results[3].session);
          EXPECT_EQ(202, service.user_id(results[3].session));
        });

  // The service has sent the request and is waiting for a response. Let's
  // register more clients to ensure the service does not mix its requests.
  const bim::server::create_session_result session_4 =
      service.create_or_refresh_session(
          address_4, 444,
          bim::net::session_token(session_token_4.begin(),
                                  session_token_4.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_4.state);

  const bim::server::create_session_result session_1_bis =
      service.create_or_refresh_session(
          address_1, 111,
          bim::net::session_token(session_token_1.begin(),
                                  session_token_1.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_1_bis.state);

  // The response for the first batch.
  last_http_request->result_handler(iscool::http::response{ 200, R"(
{
  "tokens": [ 222, 111, 333, 2222 ],
  "user_ids": [ 202, 101, null, 202 ]
}
)" });

  ASSERT_TRUE(sessions_ready);

  EXPECT_EQ(0, service.user_id(session_4.session));
  EXPECT_EQ(101, service.user_id(session_1_bis.session));

  sessions_ready = false;
  connection.disconnect();
  connection = service.connect_to_sessions_ready(
      [&](std::span<const bim::server::create_session_result> results)
        {
          sessions_ready = true;
          ASSERT_EQ(1, results.size());

          EXPECT_EQ(bim::server::create_session_result_state::accepted,
                    results[0].state);
          EXPECT_EQ(444, results[0].token);
          EXPECT_EQ(5, results[0].session);
          EXPECT_EQ(404, service.user_id(results[0].session));
        });

  last_http_request = std::nullopt;
  scheduler.tick(std::chrono::seconds(1));
  ASSERT_TRUE(!!last_http_request);
  EXPECT_EQ("biz/gs/user-id", last_http_request->url);

  body = iscool::json::parse_string(last_http_request->body);
  EXPECT_TRUE(body.isObject());
  ASSERT_EQ(1, body["sessions"].size());
  EXPECT_EQ(session_token_4, body["sessions"][0].asString());

  ASSERT_EQ(1, body["tokens"].size());
  EXPECT_EQ(444, body["tokens"][0].asInt());

  // The response for the second batch.
  last_http_request->result_handler(iscool::http::response{ 200, R"(
{
  "tokens": [ 444 ],
  "user_ids": [ 404 ]
}
)" });

  ASSERT_TRUE(sessions_ready);
}

TEST(session_service, user_id_error)
{
  bim::server::tests::fake_scheduler scheduler;
  std::optional<iscool::http::request> last_http_request;

  const iscool::http::scoped_http_delegate http(
      [&](iscool::http::request r) -> void
        {
          last_http_request = std::move(r);
        });

  bim::server::config config = bim::server::tests::new_test_config();
  config.session_clean_up_interval = std::chrono::seconds(1);
  config.session_removal_delay = std::chrono::seconds(5);
  config.business_url = "biz/";

  bim::server::statistics_service statistics(config);
  bim::server::session_service service(config, statistics);

  // Pretend some users want to connect.
  const boost::asio::ip::address_v4 address_1(0x01010101);
  const boost::asio::ip::address_v4 address_2(0x02020202);

  const std::string session_token_1 = "st1";
  const std::string session_token_2 = "st2";

  const bim::server::create_session_result session_1 =
      service.create_or_refresh_session(
          address_1, 111,
          bim::net::session_token(session_token_1.begin(),
                                  session_token_1.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_1.state);

  // The service should have queued the requests to send them all at once to
  // the business server. Updating the scheduler should trigger the query to
  // the business.
  scheduler.tick(std::chrono::seconds(1));
  ASSERT_TRUE(!!last_http_request);
  EXPECT_EQ("biz/gs/user-id", last_http_request->url);

  const Json::Value body = iscool::json::parse_string(last_http_request->body);
  EXPECT_TRUE(body.isObject());
  ASSERT_EQ(1, body["sessions"].size());
  EXPECT_EQ(session_token_1, body["sessions"][0].asString());

  ASSERT_EQ(1, body["tokens"].size());
  EXPECT_EQ(111, body["tokens"][0].asInt());

  bool sessions_ready = false;
  iscool::signals::connection connection = service.connect_to_sessions_ready(
      [&](std::span<const bim::server::create_session_result>)
        {
          sessions_ready = true;
        });

  // The service has sent the request and is waiting for a response. Let's
  // register more clients to ensure the service does not mix its requests.
  const bim::server::create_session_result session_2 =
      service.create_or_refresh_session(
          address_2, 222,
          bim::net::session_token(session_token_2.begin(),
                                  session_token_2.end()));
  EXPECT_EQ(bim::server::create_session_result_state::pending,
            session_2.state);

  // The response for the first batch.
  last_http_request->result_handler(iscool::http::response{ 500, "" });

  ASSERT_FALSE(sessions_ready);

  EXPECT_EQ(0, service.user_id(session_1.session));

  connection.disconnect();
  connection = service.connect_to_sessions_ready(
      [&](std::span<const bim::server::create_session_result> results)
        {
          sessions_ready = true;
          ASSERT_EQ(1, results.size());

          EXPECT_EQ(bim::server::create_session_result_state::accepted,
                    results[0].state);
          EXPECT_EQ(222, results[0].token);
          EXPECT_EQ(2, results[0].session);
          EXPECT_EQ(202, service.user_id(results[0].session));
        });

  last_http_request = std::nullopt;
  scheduler.tick(std::chrono::seconds(1));
  ASSERT_TRUE(!!last_http_request);
  EXPECT_EQ("biz/gs/user-id", last_http_request->url);

  // The response for the second batch.
  last_http_request->result_handler(iscool::http::response{ 200, R"(
{
  "tokens": [ 222 ],
  "user_ids": [ 202 ]
}
)" });

  ASSERT_TRUE(sessions_ready);
}
