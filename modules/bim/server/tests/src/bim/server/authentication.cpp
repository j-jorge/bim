// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/server.hpp>

#include <bim/net/message/authentication.hpp>
#include <bim/net/message/authentication_ko.hpp>
#include <bim/net/message/authentication_ok.hpp>
#include <bim/net/message/protocol_version.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/manual_scheduler.hpp>
#include <iscool/schedule/setup.hpp>
#include <iscool/signals/scoped_connection.hpp>
#include <iscool/time/setup.hpp>

#include <optional>

#include <gtest/gtest.h>

class authentication_test : public testing::Test
{
public:
  authentication_test();

protected:
  void test_full_exchange(const bim::net::authentication& message);

private:
  void interpret_received_message(bim::net::client_token token,
                                  const iscool::net::message& message);

protected:
  iscool::log::scoped_initializer m_log;
  bim::server::tests::fake_scheduler m_scheduler;

  const bim::server::config m_config;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;
  iscool::net::message_channel m_message_channel;

  std::optional<bim::net::authentication_ok> m_answer_ok;
  std::optional<bim::net::authentication_ko> m_answer_ko;
};

authentication_test::authentication_test()
  : m_config(bim::server::tests::new_test_config())
  , m_server(m_config)
  , m_socket_stream("localhost:" + std::to_string(m_config.port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_message_channel(m_message_stream, 0, 0)
{}

/// Send a message until we get an answer.
void authentication_test::test_full_exchange(
    const bim::net::authentication& message)
{
  const iscool::signals::scoped_connection connection =
      m_message_channel.connect_to_message(
          std::bind(&authentication_test::interpret_received_message, this,
                    message.get_request_token(), std::placeholders::_2));

  iscool::net::message m;
  message.build_message(m);

  for (int i = 0; (i != 10) && !m_answer_ok && !m_answer_ko; ++i)
    {
      m_message_channel.send(m);
      m_scheduler.tick(std::chrono::seconds(1));
    }
}

void authentication_test::interpret_received_message(
    bim::net::client_token token, const iscool::net::message& message)
{
  switch (message.get_type())
    {
    case bim::net::message_type::authentication_ok:
      {
        std::optional<bim::net::authentication_ok> answer =
            bim::net::try_deserialize_message<bim::net::authentication_ok>(
                message);

        if (answer && (answer->get_request_token() == token))
          m_answer_ok = std::move(*answer);
        break;
      }
    case bim::net::message_type::authentication_ko:
      {
        std::optional<bim::net::authentication_ko> answer =
            bim::net::try_deserialize_message<bim::net::authentication_ko>(
                message);

        if (answer && (answer->get_request_token() == token))
          m_answer_ko = std::move(*answer);
        break;
      }
    }
}

TEST_F(authentication_test, ok)
{
  // A valid request should be accepted, the token must match.

  const bim::net::client_token token = 1;
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());
}

TEST_F(authentication_test, bad_protocol)
{
  // A request with an invalid protocol should be refused, the token must
  // match.

  const bim::net::client_token token = 2;
  test_full_exchange(
      bim::net::authentication(2 * bim::net::protocol_version + 1, token));

  EXPECT_FALSE(!!m_answer_ok);
  ASSERT_TRUE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ko->get_request_token());
  EXPECT_EQ(bim::net::authentication_error_code::bad_protocol,
            m_answer_ko->get_error_code());
}

TEST_F(authentication_test, same_token_same_session)
{
  // Reusing the same token should produce the same session.

  // Log in with a given token.
  const bim::net::client_token token = 3;
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  // Log in again, with the same token.
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token, m_answer_ok->get_request_token());
  EXPECT_EQ(session, m_answer_ok->get_session_id());
}

TEST_F(authentication_test, different_token_different_session)
{
  // Using different tokens should produce different sessions.

  // Log in with a given token.
  const bim::net::client_token token_1 = 4;
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token_1));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  EXPECT_EQ(token_1, m_answer_ok->get_request_token());

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  // Log in with another token.
  const bim::net::client_token token_2 = 5;
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token_2));

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
  const bim::net::client_token token = 8;
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  const iscool::net::session_id session = m_answer_ok->get_session_id();
  m_answer_ok = std::nullopt;

  // Simulate inactivity. This is the default timeout on the server. There may
  // be pending UDP messages that will refresh the session, so we run many
  // times in the hope that all messages are consumed in the end *and* that
  // enough time elapses with no message to trigger the disconnection.
  for (int i = 0; i != 100; ++i)
    m_scheduler.tick(std::chrono::minutes(1));

  // Log in again with the same token.
  test_full_exchange(
      bim::net::authentication(bim::net::protocol_version, token));

  ASSERT_TRUE(!!m_answer_ok);
  EXPECT_FALSE(!!m_answer_ko);

  // The client has been disconnected, its session should be different.
  EXPECT_NE(session, m_answer_ok->get_session_id());
}
