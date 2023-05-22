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

#include <bm/net/message/authentication.hpp>
#include <bm/net/message/authentication_ko.hpp>
#include <bm/net/message/authentication_ok.hpp>
#include <bm/net/message/protocol_version.hpp>

#include <iscool/log/setup.h>
#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.h>
#include <iscool/schedule/manual_scheduler.h>
#include <iscool/schedule/setup.h>
#include <iscool/signals/scoped_connection.h>
#include <iscool/time/setup.h>

#include <optional>

#include <gtest/gtest.h>

class authentication_test : public testing::Test
{
public:
  authentication_test();

protected:
  void test_full_exchange(const bm::net::authentication& message);
  void tick(std::chrono::milliseconds duration);

protected:
  std::chrono::nanoseconds m_current_date;
  iscool::time::scoped_time_source_delegate m_time_source_initializer;
  iscool::log::scoped_initializer m_log;
  iscool::schedule::manual_scheduler m_scheduler;
  iscool::schedule::scoped_scheduler_delegate m_scheduler_initializer;

  const unsigned short m_port;
  bm::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;
  iscool::net::message_channel m_message_channel;
  iscool::net::message_deserializer m_message_deserializer;

  std::optional<bm::net::authentication_ok> m_answer_ok;
  std::optional<bm::net::authentication_ko> m_answer_ko;
};

authentication_test::authentication_test()
  : m_current_date{}
  , m_time_source_initializer(
        [this]() -> std::chrono::nanoseconds
        {
          return m_current_date;
        })
  , m_scheduler_initializer(m_scheduler.get_delayed_call_delegate())
  , m_port(10001)
  , m_server(m_port)
  , m_socket_stream("localhost:" + std::to_string(m_port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_message_channel(m_message_stream, 0, 0)
{
  m_message_channel.connect_to_message(std::bind(
      &iscool::net::message_deserializer::interpret_received_message,
      &m_message_deserializer, std::placeholders::_1, std::placeholders::_2));
}

/// Send a message until we get an answer.
void authentication_test::test_full_exchange(
    const bm::net::authentication& message)
{
  const bm::net::client_token token = message.get_request_token();

  const iscool::signals::scoped_connection connection_ok
      = m_message_deserializer.connect_signal<bm::net::authentication_ok>(
          [this, token](const iscool::net::endpoint&,
                        bm::net::authentication_ok answer) -> void
          {
            if (answer.get_request_token() != token)
              return;

            m_answer_ok = std::move(answer);
          });

  const iscool::signals::scoped_connection connection_ko
      = m_message_deserializer.connect_signal<bm::net::authentication_ko>(
          [this, token](const iscool::net::endpoint&,
                        bm::net::authentication_ko answer) -> void
          {
            if (answer.get_request_token() != token)
              return;

            m_answer_ko = std::move(answer);
          });

  for (int i = 0; (i != 100) && !m_answer_ok && !m_answer_ko; ++i)
    {
      m_message_channel.send(message.build_message());
      tick(std::chrono::seconds(1));
    }
}

void authentication_test::tick(std::chrono::milliseconds duration)
{
  std::this_thread::sleep_for(std::chrono::nanoseconds(1));
  m_current_date += duration;
  m_scheduler.update_interval(duration);
}

TEST_F(authentication_test, ok)
{
  // A valid request should be accepted, the token must match.

  const bm::net::client_token token = 1;
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());
}

TEST_F(authentication_test, bad_protocol)
{
  // A request with an invalid protocol should be refused, the token must
  // match.

  const bm::net::client_token token = 2;
  test_full_exchange(
      bm::net::authentication(2 * bm::net::protocol_version + 1, token));

  EXPECT_FALSE(!!m_answer_ok);
  ASSERT_TRUE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ko->get_request_token());
  EXPECT_EQ(bm::net::authentication_error_code::bad_protocol,
            m_answer_ko->get_error_code());
}

TEST_F(authentication_test, same_token_same_session)
{
  // Reusing the same token should produce the same session.

  // Log in with a given token.
  const bm::net::client_token token = 3;
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  // Log in again, with the same token.
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());
  EXPECT_EQ(session, m_answer_ok->get_session_id());
}

TEST_F(authentication_test, different_token_different_session)
{
  // Using different tokens should produce different sessions.

  // Log in with a given token.
  const bm::net::client_token token_1 = 4;
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token_1));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token_1, m_answer_ok->get_request_token());

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  // Log in with another token.
  const bm::net::client_token token_2 = 5;
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token_2));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token_2, m_answer_ok->get_request_token());
  EXPECT_NE(session, m_answer_ok->get_session_id());
}

TEST_F(authentication_test, client_disconnect)
{
  // The client should be disconnected after a long time without
  // activity. Disconnection is detected by a change of session obtained with
  // the same token.

  // Log in with this token.
  const bm::net::client_token token = 8;
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  // Simulate inactivity. This is the default timeout on the server. There may
  // be pending UDP messages that will refresh the session, so we run many
  // times in the hope that all messages are consumed in the end *and* that
  // enough time elapses with no message to trigger the disconnection.
  for (int i = 0; i != 100; ++i)
    tick(std::chrono::minutes(1));

  // Log in again with the same token.
  test_full_exchange(
      bm::net::authentication(bm::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  // The client has been disconnected, its session should be different.
  EXPECT_NE(session, m_answer_ok->get_session_id());
}
