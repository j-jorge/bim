/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bm/server/server.hpp>

#include <bm/net/exchange/authentication_exchange.hpp>
#include <bm/net/message/protocol_version.hpp>

#include <iscool/schedule/manual_scheduler.h>
#include <iscool/schedule/setup.h>

#include <iscool/log/setup.h>

#include <optional>

#include <gtest/gtest.h>

class authentication_test : public testing::Test
{
public:
  authentication_test();

protected:
  void test_full_exchange(iscool::net::session_id session,
                          iscool::net::channel_id channel,
                          const bm::net::authentication& message);

  void create_exchange(iscool::net::session_id session,
                       iscool::net::channel_id channel);
  void send(const bm::net::authentication& message);

protected:
  iscool::log::scoped_initializer m_log;
  iscool::schedule::manual_scheduler m_scheduler;
  iscool::schedule::scoped_scheduler_delegate m_scheduler_initializer;

  const unsigned short m_port;
  bm::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::unique_ptr<bm::net::authentication_exchange> m_exchange;
  std::optional<bm::net::authentication_ok> m_answer_ok;
  std::optional<bm::net::authentication_ko> m_answer_ko;
};

authentication_test::authentication_test()
  : m_scheduler_initializer(m_scheduler.get_delayed_call_delegate())
  , m_port(10001)
  , m_server(m_port)
  , m_socket_stream("localhost:" + std::to_string(m_port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
{}

void authentication_test::test_full_exchange(
    iscool::net::session_id session, iscool::net::channel_id channel,
    const bm::net::authentication& message)
{
  create_exchange(session, channel);
  send(message);
}

void authentication_test::create_exchange(iscool::net::session_id session,
                                          iscool::net::channel_id channel)
{
  m_exchange.reset(new bm::net::authentication_exchange(m_message_stream,
                                                        session, channel));
}

void authentication_test::send(const bm::net::authentication& message)
{
  m_exchange->connect_to_ok(
      [this](const iscool::net::endpoint&,
             bm::net::authentication_ok answer) -> void
      {
        m_answer_ok = std::move(answer);
        m_exchange->stop();
      });

  m_exchange->connect_to_ko(
      [this](const iscool::net::endpoint&,
             bm::net::authentication_ko answer) -> void
      {
        m_answer_ko = std::move(answer);
        m_exchange->stop();
      });

  m_exchange->start(message);

  for (int i = 0; (i != 10) && !m_answer_ok && !m_answer_ko; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      m_scheduler.update_interval(std::chrono::seconds(1));
    }
}

TEST_F(authentication_test, ok)
{
  const bm::net::client_token token = 1;
  test_full_exchange(
      0, 0, bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());
}

TEST_F(authentication_test, bad_protocol)
{
  const bm::net::client_token token = 2;
  test_full_exchange(
      0, 0, bm::net::authentication(2 * bm::net::protocol_version + 1, token));

  EXPECT_FALSE(!!m_answer_ok);
  ASSERT_TRUE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ko->get_request_token());
  EXPECT_EQ(bm::net::authentication_error_code::bad_protocol,
            m_answer_ko->get_error_code());
}

TEST_F(authentication_test, same_token_same_session)
{
  const bm::net::client_token token = 3;
  test_full_exchange(
      0, 0, bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  const iscool::net::session_id session = m_answer_ok->get_session_id();

  m_answer_ok = std::nullopt;

  send(bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(session, m_answer_ok->get_session_id());
}

TEST_F(authentication_test, different_token_different_session)
{
  test_full_exchange(0, 0,
                     bm::net::authentication(bm::net::protocol_version, 4));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  const iscool::net::session_id session = m_answer_ok->get_session_id();

  m_answer_ok = std::nullopt;

  send(bm::net::authentication(bm::net::protocol_version, 5));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_NE(session, m_answer_ok->get_session_id());
}

TEST_F(authentication_test, DISABLED_client_disconnect)
{
  const bm::net::client_token token = 5;
  test_full_exchange(
      0, 0, bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  m_scheduler.update_interval(std::chrono::hours(1));

  send(bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  // The client has been disconnected, its session should be different.
  EXPECT_NE(session, m_answer_ok->get_session_id());
}
